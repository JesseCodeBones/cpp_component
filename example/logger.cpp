#include "../src/logger/logger.hpp"
#include <map>
#include <unordered_map>
#include <vector>

int main() {
  // Logger::setLoggerPostion("1.log");
  Logger::logInfo(1, 12, "jjj");
  Logger::logError(std::vector<int>{1, 2, 3}, std::vector<float>{1.1, 1.2});

  std::unordered_map<int, std::vector<int>> map{};
  map[1] = {1, 23, 4};
  map[20] = {1, 23, 4, 5, 10};
  Logger::logDebug(map);
}