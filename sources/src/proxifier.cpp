#include "proxifier.hpp"

Proxifier::Proxifier(proxy_t proxy)
{
	Proxifier::init_wsa();
	this->proxy = proxy;
}

char					*Proxifier::get_last_error()
{
	int					message_id;
	char				*result(NULL);

	message_id = WSAGetLastError();
	if (message_id == 0)
		return result;

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, message_id,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&result, 0, NULL);
	return result;
}

bool					Proxifier::connect(string remote_host, uint16_t remote_port)
{
	/*char				buffer[512 + 1];*/
	SOCKADDR_IN			sockAddr;
	int					size(0);

	/* Setup the socket */
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockAddr.sin_addr.s_addr = inet_addr(remote_host.c_str());
	sockAddr.sin_port = htons(remote_port);
	sockAddr.sin_family = AF_INET;

	/* Trying to connect */
	if (::connect(this->sock, (SOCKADDR*)(&sockAddr), sizeof(sockAddr)) == SOCKET_ERROR)
		return false;

	/* Generate and send packet */
	if (send(generate_packet(remote_host, remote_port)) == SOCKET_ERROR)
		return false;

	/* Get response */
	Sleep(50);
	/*while (size <= 0)
	{
		size = recv(this->sock, buffer, 512, 0);
		if (size <= 0)
		{
			Sleep(50);
			continue;
		}

		buffer[size] = 0;
		this->last_response = parse_response(buffer);
	}*/
	return true;
}

int						Proxifier::send(string packet)
{
	return ::send(this->sock, packet.c_str(), packet.size(), 0);
}

void					Proxifier::close()
{
	closesocket(this->sock);
}

response_t				Proxifier::get_last_response()
{
	return this->last_response;
}

vector<proxy_t>			Proxifier::proxies_from_file(string filename)
{
	size_t				size;
	string				content;
	stringstream		tmp;
	vector<proxy_t>		result;
	vector<string>		tmp_split;
	vector<string>		tmp_proxy;
	ifstream			file(filename, ios::in | ios::ate);

	if (!file.is_open())
	{
		tmp << "[!] Unable to open the file '" << filename << "'";
		throw std::logic_error(tmp.str());
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

void					Proxifier::init_wsa()
{
	WSADATA				wsaData;
	static bool			init_done(false);

	if (init_done)
		return;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	init_done = true;
}

response_t				Proxifier::parse_response(const char * buffer)
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

string					Proxifier::generate_packet(string remote_host, uint16_t remote_port)
{
	stringstream		packet;

	packet << "CONNECT " << remote_host << ":" << remote_port << " HTTP/1.1\r\n";
	packet << "Host: " << remote_host << ":" << remote_port << "\r\n";
	packet << "\r\n";
	return packet.str();
}
