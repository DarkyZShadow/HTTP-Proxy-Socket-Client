#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#define PROXY_HOST "139.255.40.130"
#define PROXY_PORT 8080
#define REMOTE_HOST "79.110.84.75"
#define REMOTE_PORT 4002

int					main(int argc, char **argvs)
{
	stringstream	packet;

	packet << "CONNECT " << REMOTE_HOST << ":" << REMOTE_PORT << " HTTP/1.1\r\n";
	packet << "HOST " << REMOTE_HOST << ":" << REMOTE_PORT << "\r\n";
	packet << "\r\n";

	cout << packet.str() << endl;

	system("PAUSE");
	return 0;
}