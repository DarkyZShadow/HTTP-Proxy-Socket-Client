#include "string_funcs.hpp"

vector<string> split(std::string s, char delim)
{
	std::string item;
	std::stringstream ss;
	vector<string> result;

	ss.str(s);
	while (std::getline(ss, item, delim))
		result.push_back(item);
	return result;
}

bool					starts_with(std::string str, std::string key)
{
	return (strncmp(&str[0], &key[0], strlen(&key[0])) == 0);
}
