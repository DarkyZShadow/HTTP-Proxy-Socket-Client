#pragma once
#ifndef __STRING_FUNCS_HPP__
#define __STRING_FUNCS_HPP__

#include <string>
#include <vector>
#include <sstream>

using namespace std;

vector<string>			split(std::string s, char delim);
bool					starts_with(std::string str, std::string key);

#endif
