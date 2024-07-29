#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

std::mutex set_lock;
std::unordered_set<std::string> unique_words;

void append_to_set(const std::unordered_set<std::string> set) {
    std::lock_guard<std::mutex> lock(set_lock);
    unique_words.insert(set.begin(), set.end());
}

unsigned int get_file_size(std::ifstream& file) {
    std::streampos begin, end;
    begin = file.tellg();
    file.seekg(0, std::ifstream::end);
    end = file.tellg();
    file.seekg(0, std::ifstream::beg);
    return end - begin;
}

unsigned int get_chunk_size(std::ifstream& file) {
    const unsigned int filesize{get_file_size(file)};
    unsigned int processor_count = std::thread::hardware_concurrency();

    if (processor_count == 0) {
        processor_count = 8;
    }

    const unsigned int chunk_size = get_file_size(file) / processor_count;
    const unsigned int prevent_additional_chunk = filesize - (chunk_size * processor_count);

    // std::cout << "pc: " << processor_count << std::endl
    //           << "fs: " << get_file_size(file) << std::endl
    //           << "pac: " << prevent_additional_chunk << std::endl;

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

    const unsigned int chunk_size{get_chunk_size(file)};
    std::cout << "b size: " << chunk_size << std::endl;

    std::vector<std::thread> threads;
    std::string buffer(chunk_size, '\0');

    while (file.read(buffer.data(), chunk_size) || file.gcount() > 0) {
        buffer.resize(file.gcount());

        // get closest whole word
        if (file.gcount() == chunk_size) {
            char c;
            while (file.get(c) && c != ' ') {
                buffer.push_back(c);
            }
        }

        std::cout << "thread " << threads.emplace_back(process_chunk, buffer).get_id() << " started" << std::endl;
        buffer.clear();
        buffer.resize(chunk_size, '\0');
    }
    file.close();

    for (std::thread& t : threads) {
        if (t.joinable()) {
            std::cout << "thread" << t.get_id() << "joined" << std::endl;
            t.join();
        }
    }

    std::cout << unique_words.size() << std::endl;

    return 0;
}
