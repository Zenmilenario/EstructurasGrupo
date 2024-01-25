#include <atomic>
#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "pel_print.hpp"

namespace fs = std::filesystem;

struct Extension_info {
    std::uintmax_t num_files, total_size;
};

using map_type = std::map<std::string, Extension_info>;

// Function to process a partial map and update the global map
void process_map(const map_type& partial_map, map_type& global_map, std::atomic<std::uintmax_t>& directory_counter) {
    // Merge the results of each thread into processed_data
    for (const auto& [dir, info] : partial_map) {
        global_map[dir].num_files += info.num_files;
        global_map[dir].total_size += info.total_size;
    }

    // Update the directory counter
    directory_counter += partial_map.size();
}

// Function to generate a partial map from immediate subdirectories of a chunk of paths
map_type generate_map(const std::vector<fs::path>& chunk) {
    map_type res;
    for (const fs::path& pth : chunk) {
        if (fs::is_directory(pth)) {
            auto dir_name = pth.filename().string();
            res[dir_name].num_files = 0;
            res[dir_name].total_size = 0;

            // Calculate the total size of the directory correctly
            try {
                for (const auto& entry : fs::directory_iterator{pth}) {
                    if (fs::is_regular_file(entry)) {
                        res[dir_name].num_files += 1;
                        res[dir_name].total_size += fs::file_size(entry);
                    }
                }
            } catch (const std::exception& e) {
                // Handle the exception (print an error message, log, etc.)
                std::cerr << "Exception caught: " << e.what() << std::endl;
            }
        }
    }

    // Debugging print statements
    for (const auto& [dir, info] : res) {
        std::cout << "Debug: " << dir << ": " << info.num_files << " files " << info.total_size << " bytes\n";
    }

    return res;
}



int main() {
    pel::print("Please insert a root: ");
    auto root_str = std::string{};
    std::getline(std::cin, root_str);

    auto const root = fs::path{root_str};

    if (!fs::is_directory(root)) {
        pel::print("you must indicate an actual directory");
        return 0;
    }

    auto const start = std::chrono::steady_clock::now();

    auto paths = std::vector<fs::path>{};
    for (fs::path pth : fs::recursive_directory_iterator{root}) {
        paths.push_back(pth);
    }

    auto processed_data = map_type{};
    auto directory_counter = std::atomic<std::uintmax_t>{0};

    auto const hardw_threads = std::thread::hardware_concurrency();
    auto const num_futures = hardw_threads - 1;
    auto const max_chunk_sz = paths.size() / hardw_threads;

    auto futures = std::vector<std::future<map_type>>{};

    // Launch threads to process chunks of paths asynchronously with exception handling
    for (std::size_t i = 0; i < num_futures; ++i) {
        auto begin = paths.begin() + i * max_chunk_sz;
        auto end = (i == num_futures - 1) ? paths.end() : begin + max_chunk_sz;

        try {
            futures.push_back(std::async(std::launch::async, generate_map, std::vector<fs::path>(begin, end)));
        } catch (const std::exception& e) {
            std::cerr << "Exception caught: " << e.what() << std::endl;
            futures.push_back(std::async([] { return map_type{}; }));
        }
    }

    // Wait for the asynchronous tasks to complete and process the results
    for (auto& future : futures) {
        process_map(future.get(), processed_data, directory_counter);
    }

    // Print the subdirectory information
    pel::println("                 -------------[Directories]------------\n");
    for (const auto& [dir, info] : processed_data) {
        //std::cout << dir << ": " << info.num_files << " files " << info.total_size << " bytes\n";
        if (info.num_files > 0) {
            pel::println("{:>25}: {:>5} files {:>10} bytes", dir, info.num_files, info.total_size);
        }    
    }
 
    auto total_subdirectories = std::accumulate(processed_data.begin(), processed_data.end(), 0,
                                                [](int sum, const auto& entry) { return sum + entry.second.num_files; });
    auto total_space = std::accumulate(processed_data.begin(), processed_data.end(), 0,
                                    [](std::uintmax_t sum, const auto& entry) { return sum + entry.second.total_size; });
 
    //pel::println("[Total]: {} files {} bytes", total_subdirectories, total_space);
 
    auto const stop = std::chrono::steady_clock::now();
    auto const duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    //pel::println("[Total directories]: {} | [Total time]: {} ms", directory_counter.load(), duration.count());
    pel::println("");
    pel::println("{:>20}Total: {:>5} files {:>10} bytes | {} folders [{} ms]", "" ,total_subdirectories, total_space, directory_counter.load(), duration.count());
}



//test