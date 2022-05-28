#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ostream>

namespace Logger {

namespace {
class Logger {
public:
  static Logger &getInstance() noexcept {
    static Logger stream{};
    return stream;
  }
  void setOutput(const char *path) {
    logger_ = new std::ofstream(path);
    if (logger_->bad()) { throw std::runtime_error("cannot open file"); }
  }

  template <typename... Args> void log(Args &&...args) {
    std::lock_guard<std::mutex> lo{log_mutex};
    auto timestamp = static_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch());
    (*logger_) << '[' << timestamp.count() << ']';
    _log(std::forward<Args>(args)...);
  }

private:
  Logger() noexcept { logger_ = &std::cerr; }
  Logger(Logger const &) = delete;
  Logger(Logger &&) = delete;
  Logger &operator=(Logger const &) = delete;
  Logger &operator=(Logger &&) = delete;
  ~Logger() {
    if (logger_ != &std::cerr) { delete logger_; }
  }

private:
  std::ostream *logger_;

  std::mutex log_mutex;

  void _log() { (*logger_) << "\n"; }
  template <typename T, typename... Args> void _log(T &&first, Args &&...args) {
    (*logger_) << std::forward<T>(first) << " ";
    _log(std::forward<Args>(args)...);
  }
};
}; // namespace

inline void setLoggerPostion(const char *path) { Logger::getInstance().setOutput(path); }

template <typename... Args> void logDebug(Args &&...args) { Logger::getInstance().log("[DEBUG]", args...); }
template <typename... Args> void logInfo(Args &&...args) { Logger::getInstance().log("[INFO]", args...); }
template <typename... Args> void logError(Args &&...args) { Logger::getInstance().log("[ERROR]", args...); }
template <typename... Args> void logWarning(Args &&...args) { Logger::getInstance().log("[WARNING]", args...); }

}; // namespace Logger
