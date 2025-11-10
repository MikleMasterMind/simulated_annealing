#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>

class Logger {
public:
    static void initialize(bool enabled, const std::string& filename = "algorithm.log");
    static void log(const std::string& message);
    static void setEnabled(bool enabled);

private:
    static bool enabled_;
    static std::ofstream logFile_;
    static std::mutex logMutex_;
};