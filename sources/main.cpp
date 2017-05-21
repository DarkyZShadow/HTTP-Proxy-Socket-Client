#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include "string_funcs.hpp"

using namespace std;

#pragma comment(lib, "ws2_32.lib")
#define PROXY_HOST "35.161.58.221"
#define PROXY_PORT 3128
/*#define REMOTE_HOST "79.110.84.75"
#define REMOTE_PORT 4002*/
#define REMOTE_HOST "138.68.169.204"
#define REMOTE_PORT 8080

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

void					connection();
string					generate_packet();
response_t				parse_response(const char *buffer);

int						main(int argc, char **argvs)
{
	connection();
	system("PAUSE");
	return 0;
}

string					generate_packet()
{
	stringstream		packet;

	packet << "CONNECT " << REMOTE_HOST << ":" << REMOTE_PORT << " HTTP/1.1\r\n";
	//packet << "Host: " << REMOTE_HOST << ":" << REMOTE_PORT << "\r\n";
	//packet << "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0\r\n";
	packet << "\r\n";
	cout << packet.str();
	return packet.str();
}

void					connection()
{
	char				buffer[512 + 1];
	SOCKADDR_IN			SockAddr;
	WSADATA				wsaData;
	SOCKET				sock;
	string				packet;
	int					size(0);
	response_t			res;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SockAddr.sin_addr.s_addr = inet_addr(PROXY_HOST);
	SockAddr.sin_port = htons(PROXY_PORT);
	SockAddr.sin_family = AF_INET;

	cout << "[ ] Trying to connect to proxy" << endl;
	if (connect(sock, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		cout << "[!] Could not connect" << endl;
		return;
	}

	/* Generate and send packet */
	packet = generate_packet();
	send(sock, packet.c_str(), packet.size(), 0);

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
		cout << "Response (" << PROXY_HOST << ":" << PROXY_PORT << ") : " << res.status_code << " - " << res.reason_phrase << endl;
	}
	
	closesocket(sock);
	WSACleanup();
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
