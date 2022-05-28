#include "../src/logger/logger.hpp"

int main() {
  // Logger::setLoggerPostion("1.log");
  Logger::logInfo(1, 12, "jjj");
  Logger::logDebug(1, 12, "jjj");
  Logger::logDebug(1, 12, "jjj");
  Logger::logDebug(1, 12, "jjj");
}