#include "Logger.h"

bool Logger::enabled_ = false;
std::ofstream Logger::logFile_;
std::mutex Logger::logMutex_;

void Logger::initialize(bool enabled, const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex_);
    enabled_ = enabled;
    
    if (enabled_) {
        logFile_.open(filename, std::ios::out | std::ios::app);
        if (logFile_.is_open()) {
            logFile_ << "=== Algorithm Log Started ===" << std::endl;
        }
    }
}

void Logger::log(const std::string& message) {
    if (!enabled_) return;
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    // Вывод в консоль
    std::cout << "[LOG] " << message << std::endl;
    
    // Запись в файл
    if (logFile_.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        logFile_ << "[" << std::ctime(&time) << "] " << message << std::endl;
    }
}

void Logger::setEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(logMutex_);
    enabled_ = enabled;
}