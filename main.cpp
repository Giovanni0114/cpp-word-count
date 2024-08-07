#include <sqlite3.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <semaphore>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

// if file is smaller than 100MB * max_threads -> chunk_size = filesize / max_threads
// if there is not enough memory -> chunk_size = available_ram * 0.8
// the chunk size should be about 100MB should be good enough for most cases
#define CHUNK_SIZE (unsigned int)(1024 * 1024 * 100)  // 100 MB

std::mutex set_lock;

std::unordered_set<std::string> unique_words;

std::random_device dev;
std::mt19937 rng(dev());

std::vector<std::thread> threads;

const unsigned int max_threads = std::thread::hardware_concurrency();
std::counting_semaphore<> semaphore(max_threads);

const long page_size{sysconf(_SC_PAGESIZE)};

// prevent multiple threads ended in about same time
//      -> this may reasult in throttling on writing to unordered_set
int random_jitter(const unsigned int size) {
    std::uniform_int_distribution<std::mt19937::result_type> dist(size * -0.2, size * 0.2);
    return dist(rng);
}

bool get_available_memory(unsigned int& size) {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        size = info.freeram;
        return true;
    }
    return false;
}

unsigned int new_chunk_size(const unsigned int original_size) {
    unsigned int new_chunk = original_size + random_jitter(original_size);
    unsigned int freeram = 0;

    if (get_available_memory(freeram)) {
        new_chunk = std::min<unsigned int>(freeram * 0.8, new_chunk);
    }

    return new_chunk - (new_chunk % page_size);
}

void append_to_set(const std::unordered_set<std::string> set) {
    std::lock_guard<std::mutex> lock(set_lock);
    unique_words.insert(set.begin(), set.end());

#ifdef DEBUG
    std::cout << "appended: " << unique_words.size() << " for " << std::this_thread::get_id() << std::endl;
#endif

    semaphore.release();
}

// quick jump to end to tell how big the file is
unsigned int get_file_size(std::ifstream& file) {
    std::streampos begin, end;
    begin = file.tellg();
    file.seekg(0, std::ifstream::end);
    end = file.tellg();
    file.seekg(0, std::ifstream::beg);
    return end - begin;
}

// calculate if the file is not too small
// this may allow use whole CPU even with smaller files
unsigned int get_chunk_size(std::ifstream& file) {
    const unsigned int filesize{get_file_size(file)};

    const unsigned int chunk_size = get_file_size(file) / max_threads;
    const unsigned int prevent_additional_chunk = filesize - (chunk_size * max_threads);

    return chunk_size + prevent_additional_chunk;
}

void process_chunk(const std::string& chunk) {
    std::unordered_set<std::string> local_set;
    std::istringstream stream(chunk);
    std::string word;

    while (stream >> word) {
        local_set.insert(word);
    }

    append_to_set(local_set);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ios::in);
    if (!file) {
        throw std::runtime_error("Unable to open file");
    }

    const unsigned int original_chunk_size{std::min<unsigned int>(get_chunk_size(file), CHUNK_SIZE)};
    unsigned int chunk_size{original_chunk_size};
    std::string buffer(chunk_size, '\0');

    while (file.read(buffer.data(), chunk_size) || file.gcount() > 0) {
        buffer.resize(file.gcount());

        {
            // get closest whole word
            char c;
            while (file.get(c) && c != ' ') {
                buffer.push_back(c);
            }
        }

        semaphore.acquire();

#ifndef DEBUG
        threads.emplace_back(process_chunk, buffer);
#else
        std::cout << "created " << threads.emplace_back(process_chunk, buffer).get_id() << " for " << chunk_size
                  << " bytes" << std::endl;
#endif

        buffer.clear();
        chunk_size = new_chunk_size(original_chunk_size);
        buffer.resize(chunk_size, '\0');
    }

    file.close();

    for (std::thread& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << unique_words.size() << std::endl;

    return 0;
}
