#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "proxifier.hpp"

#define REMOTE_HOST "138.68.169.204"
#define REMOTE_PORT 8080
/*#define REMOTE_HOST "79.110.84.75"
#define REMOTE_PORT 4002*/

using namespace std;

int						main(int argc, char **argvs)
{
	Proxifier			*proxifier;
	vector<proxy_t>		proxies;
	proxy_t				cur_proxy;
	response_t			last_res;

	setlocale(LC_ALL, "");

	/* Get proxies list from a file (don't forget to try/catch this function) */
	proxies = Proxifier::proxies_from_file("resources/working.txt");
	for (size_t i = 0; i < proxies.size(); ++i)
	{
		cur_proxy = proxies[i];
		cout << "[ ] Proxy Connection Testing (" << cur_proxy.host << ":" << cur_proxy.port << ")" << endl;

		/* Create new Proxifier */
		proxifier = new Proxifier(cur_proxy);
		if (!proxifier->connect(REMOTE_HOST, REMOTE_PORT))
		{
			cout << "[!] Error : " << proxifier->get_last_error();
			delete proxifier;
			continue;
		}

		/* Get response */
		last_res = proxifier->get_last_response();
		cout << "[ ] Response (" << cur_proxy.host << ":" << cur_proxy.port << ") : "
			 << last_res.status_code << " - " << last_res.reason_phrase << endl;
		proxifier->close();
		delete proxifier;
	}
	system("PAUSE");
	return 0;
}
