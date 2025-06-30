#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

class BulkLogger {
public:
    BulkLogger(size_t bulk_size) : bulk_size_(bulk_size), in_dynamic_block_(false) {}

    void process_command(const std::string& command) {
        if (command == "{") {
            start_dynamic_block();
        } else if (command == "}") {
            end_dynamic_block();
        } else {
            commands_.push_back(command);
            if (!in_dynamic_block_ && commands_.size() == bulk_size_) {
                flush_bulk();
            }
        }
    }

    void flush() {
        if (!commands_.empty()) {
            flush_bulk();
        }
    }

private:
    void start_dynamic_block() {
        if (!in_dynamic_block_) {
            flush_bulk(); // End current bulk
            in_dynamic_block_ = true;
        }
    }

    void end_dynamic_block() {
        if (in_dynamic_block_) {
            in_dynamic_block_ = false;
            flush_bulk(); // Process dynamic bulk
        }
    }

    void flush_bulk() {
        if (commands_.empty()) {
            return;
        }

        long long timestamp = std::chrono::system_clock::now().time_since_epoch().count() / 1000000000;
        std::string filename = "bulk" + std::to_string(timestamp) + ".log";

        std::ofstream log_file(filename);
        std::cout << "bulk: ";
        log_file << "bulk: ";

        for (size_t i = 0; i < commands_.size(); ++i) {
            std::cout << commands_[i];
            log_file << commands_[i];
            if (i < commands_.size() - 1) {
                std::cout << ", ";
                log_file << ", ";
            }
        }
        std::cout << std::endl;
        log_file << std::endl;
        log_file.close();

        commands_.clear();
    }

private:
    size_t bulk_size_;
    bool in_dynamic_block_;
    std::vector<std::string> commands_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: bulk <bulk_size>" << std::endl;
        return 1;
    }

    size_t bulk_size;
    try {
        bulk_size = std::stoul(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid bulk size: " << argv[1] << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Bulk size out of range: " << argv[1] << std::endl;
        return 1;
    }

    BulkLogger logger(bulk_size);
    std::string command;

    while (std::getline(std::cin, command)) {
        logger.process_command(command);
    }

    logger.flush(); // Flush any remaining commands

    return 0;
}