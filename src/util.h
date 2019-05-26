#ifndef UTIL_H_
#define UTIL_H_

#include <fstream>
#include <string>

inline std::string get_process_name() {
  auto ifs = std::ifstream("/proc/self/comm");
  auto str = std::string{};
  std::getline(ifs, str);
  return str;
}

#endif /* UTIL_H_ */
