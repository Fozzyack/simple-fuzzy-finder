#pragma once
#include <vector>
#include <string>

void search_paths (const std::vector<std::string> dirs, const std::string &search, std::vector<std::string> &result);

std::string render(const std::vector<std::string> &result, int &choice, int entries);
