#include "../src/fix_length_list/fix_length_list.hpp"
#include <cstdint>
#include <iostream>
#include <string>

int main() {
  FixLengthList::FixLengthList<std::string, 10> list{};
  list.emplace_back("1");
  std::cout << list << "\n";
  list.emplace_back("2");
  std::cout << list << "\n";
  list.emplace_back("3");
  std::cout << list << "\n";
  list.emplace_back("4");
  std::cout << list << "\n";
  list.emplace_front("0");
  std::cout << list << "\n";
  list.emplace_front("-1");
  std::cout << list << "\n";
  list.pop_back();
  std::cout << list << "\n";
  list.pop_back();
  std::cout << list << "\n";
  list.pop_front();
  std::cout << list << "\n";
  list.pop_front();
  std::cout << list << "\n";
  return 0;
}