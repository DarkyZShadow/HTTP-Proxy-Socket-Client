#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")
#define PROXY_HOST "107.170.211.224"
#define PROXY_PORT 8080
#define REMOTE_HOST "79.110.84.75"
#define REMOTE_PORT 4002

void					connection();
string					generate_packet();

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
	packet << "HOST " << REMOTE_HOST << ":" << REMOTE_PORT << "\r\n";
	packet << "\r\n";
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
		cout << buffer << endl;
	}

	closesocket(sock);
	WSACleanup();
}
