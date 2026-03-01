#pragma once

#include <cstdint>
#include <string>

bool ParsePasvResponse(const std::string& response, std::string& ip, uint16_t& port);
