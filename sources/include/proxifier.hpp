﻿#pragma once
#ifndef __PROXIFIER_HPP__
#define __PROXIFIER_HPP__
#ifndef INFINITE
#define INFINITE UINT32_MAX
#endif

#include <algorithm>			/* std::remove */
#include <Windows.h>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <map>
#include "string_funcs.hpp"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

typedef struct
{
	string						host;
	uint16_t					port;
} proxy_t;

typedef struct
{
	map<string, string>			headers;
	string						http_version;
	string						reason_phrase;
	string						body;
	int							status_code;
} response_t;

class Proxifier
{
	private:
		uint32_t				time_out;
		string					last_error;

	private:
		SOCKET					sock;
		proxy_t					proxy;
		response_t				last_response;

	public:
		Proxifier(proxy_t proxy, uint32_t time_out = 15000);
		string					get_last_error();
		bool					connect(string remote_host, uint16_t remote_port);
		int						send(string packet);
		string					recv(uint32_t time_out = INFINITE);
		void					close();
		response_t				get_last_response();
		static vector<proxy_t>	proxies_from_file(string filename);

	private:
		static void				init_wsa();
		static response_t		parse_response(const char *buffer);
		static string			generate_packet(string remote_host, uint16_t remote_port);
};

#endif /* __PROXIFIER_HPP__ */
