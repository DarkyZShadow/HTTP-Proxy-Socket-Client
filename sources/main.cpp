#include <algorithm>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "string_funcs.hpp"

using namespace std;

#pragma comment(lib, "ws2_32.lib")

typedef struct
{
	string				host;
	uint16_t			port;
} proxy_t;

typedef struct
{
	map<string, string>	headers;
	string				http_version;
	string				reason_phrase;
	string				body;
	int					status_code;
} response_t;

void					init_wsa();
vector<proxy_t>			get_proxies(string filename);
response_t				parse_response(const char *buffer);
string					generate_packet(string remote_host, uint16_t remote_port);
void					connection(proxy_t proxy, string remote_host, uint16_t remote_port);

int						main(int argc, char **argvs)
{
	vector<proxy_t>		proxies;
	string				remote_host("138.68.169.204");
	uint16_t			remote_port(8080);

	init_wsa();
	proxies = get_proxies("resources/proxylist2.txt");
	for (size_t i = 0; i < proxies.size(); ++i)
		connection(proxies[i], remote_host, remote_port);
	
	system("PAUSE");
	return 0;
}

vector<proxy_t>			get_proxies(string filename)
{
	size_t				size;
	string				content;
	vector<proxy_t>		result;
	vector<string>		tmp_split;
	vector<string>		tmp_proxy;
	ifstream			file(filename, ios::in | ios::ate);

	if (!file.is_open())
	{
		cout << "[!] Unable to open the file '" << filename << "'" << endl;
		return result;
	}

	/* Read the file */
	size = (size_t)file.tellg();
	content.resize(size);
	file.seekg(0, ios::beg);
	file.read(&content[0], size);
	file.close();

	/* Perform operations on content */
	content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());
	tmp_split = split(content, '\n');
	for (size_t i = 0; i < tmp_split.size(); ++i)
	{
		if ('#' == tmp_split[i][0] || tmp_split[i].size() == 0)
			continue;
		if ((tmp_proxy = split(tmp_split[i], ':')).size() != 2)
			continue;
		result.push_back({ tmp_proxy[0], (uint16_t)atoi(tmp_proxy[1].c_str()) });
	}
	return result;
}

string					generate_packet(string remote_host, uint16_t remote_port)
{
	stringstream		packet;

	packet << "CONNECT " << remote_host << ":" << remote_port << " HTTP/1.1\r\n";
	packet << "Host: " << remote_host << ":" << remote_port << "\r\n";
	//packet << "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0\r\n";
	packet << "\r\n";
	return packet.str();
}

void					init_wsa()
{
	WSADATA				wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void					connection(proxy_t proxy, string remote_host, uint16_t remote_port)
{
	char				buffer[512 + 1];
	SOCKADDR_IN			SockAddr;
	SOCKET				sock;
	string				packet;
	int					size(0);
	int					error;
	response_t			res;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SockAddr.sin_addr.s_addr = inet_addr(remote_host.c_str());
	SockAddr.sin_port = htons(remote_port);
	SockAddr.sin_family = AF_INET;

	cout << "[ ] Trying to connect to proxy" << endl;
	if (connect(sock, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		cout << "[!] Could not connect : " << WSAGetLastError() << endl;
		return;
	}

	/* Generate and send packet */
	packet = generate_packet(remote_host, remote_port);
	error = send(sock, packet.c_str(), packet.size(), 0);

	/* Get response */
	while (size <= 0)
	{
		size = recv(sock, buffer, 512, 0);
		if (size <= 0)
		{
			Sleep(50);
			continue;
		}

		buffer[size] = 0;
		res = parse_response(buffer);
		cout << "Response (" << proxy.host << ":" << proxy.port << ") : " << res.status_code << " - " << res.reason_phrase << endl;
	}
	closesocket(sock);
}

response_t				parse_response(const char *buffer)
{
	size_t				limit;
	size_t				tmp_limit;
	string				tmp;
	string				headers;
	response_t			result;
	
	/* Get headers */
	limit = strstr(buffer, "\r\n\r\n") - buffer;
	headers.resize(limit);
	strncpy(&headers[0], buffer, limit);

	/* Set body */
	tmp_limit = strlen(buffer) - limit - 4;
	result.body.resize(tmp_limit);
	strncpy(&result.body[0], &buffer[limit + 4], tmp_limit);

	/* Get status info */
	limit = headers.find(" ");
	result.http_version = headers.substr(0, limit);
	headers.erase(0, limit + 1);
	limit = headers.find(" ");
	result.status_code = atoi(&headers.substr(0, limit)[0]);
	headers.erase(0, limit + 1);
	limit = headers.find("\r\n");
	if (limit == string::npos)
		limit = headers.end() - headers.begin();
	result.reason_phrase = headers.substr(0, limit);
	headers.erase(0, limit + 2);
	
	/* Parse headers */
	limit = 0;
	while (limit != string::npos)
	{
		limit = headers.find("\r\n");
		if (limit == string::npos)
			tmp = headers;
		else
			tmp = headers.substr(0, limit);
		headers.erase(0, limit + 2);

		tmp_limit = tmp.find(": ");
		if (tmp_limit == string::npos)
			continue;
		result.headers.insert({ tmp.substr(0, tmp_limit), tmp.substr(tmp_limit + 2) });
	}
	return result;
}
