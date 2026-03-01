#include "FtpUtils.h"

#include <cstdlib>
#include <sstream>
#include <vector>

bool ParsePasvResponse(const std::string& response, std::string& ip, uint16_t& port) {
  const std::size_t l = response.find('(');
  const std::size_t r = response.find(')', l == std::string::npos ? 0 : l + 1);
  if (l == std::string::npos || r == std::string::npos || r <= l + 1) return false;

  std::string body = response.substr(l + 1, r - l - 1);
  std::vector<int> nums;
  std::stringstream ss(body);
  std::string token;
  while (std::getline(ss, token, ',')) {
    if (token.empty()) return false;
    char* end = nullptr;
    long value = std::strtol(token.c_str(), &end, 10);
    if (*end != '\0' || value < 0 || value > 255) return false;
    nums.push_back(static_cast<int>(value));
  }

  if (nums.size() != 6) return false;
  ip = std::to_string(nums[0]) + "." + std::to_string(nums[1]) + "." +
       std::to_string(nums[2]) + "." + std::to_string(nums[3]);
  port = static_cast<uint16_t>(nums[4] * 256 + nums[5]);
  return true;
}
