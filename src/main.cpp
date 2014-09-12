#include <ev++.h>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>

using namespace std;

#include "httpparser.hpp"

int main(){
	int port = 9000;
	char buffer[1024];
	init_http_parser();
	printf("Listening on port %d\n", port);
	struct sockaddr_in server_addr;
	int s = socket(PF_INET, SOCK_STREAM, 0);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
	
	if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		cout<<"binding error"<<endl;
		return 1;
	}
	
	listen(s, 1);
	
	int client; size_t clen;
	struct sockaddr_in c_addr;
	
	while (1){
		if ((client = accept(s, (struct sockaddr *) &c_addr, &clen)) < 0)
			return 1;
		
		ssize_t nread = recv(client, buffer, sizeof(buffer), 0);
		if (nread > 0){
			string strreq(buffer, nread);
			
			map<string, string> req = get_request(strreq);
			
			cout << req["method"] << " " << req["url"] << endl;
			
			string response = build_response(200, "text/html", "hello world!");
			
			ssize_t written = write(client, response.c_str(), response.length());
		}
		
		close(client);
	}
}