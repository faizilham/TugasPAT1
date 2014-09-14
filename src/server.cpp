#include "server.hpp"

ClientHandler::ClientHandler (int sock) : client_sock(sock){
	// add non blocking flag
	fcntl(client_sock, F_SETFL, fcntl(client_sock, F_GETFL, 0) | O_NONBLOCK); 
	
	printf("Clients: %d\n", ++numclient);
	
	io.set<ClientHandler, &ClientHandler::callback>(this); // set callback
	io.start(client_sock, ev::READ); // start listening read
}

ClientHandler::~ClientHandler(){
	io.stop();
	close(client_sock);
	--numclient;
}

void ClientHandler::callback(ev::io &watcher, int revents){
	if (EV_ERROR & revents) {
		printf("Error: Invalid event");
		return;
	}

	if (revents & EV_READ) 
		read_cb(watcher);

	if (revents & EV_WRITE) 
		write_cb(watcher);

	if (response_queue.empty()) {
		io.set(ev::READ); // may only read
	} else {
		io.set(ev::READ|ev::WRITE); // may read and write
	}
}

void ClientHandler::write_cb(ev::io &watcher){
	if (response_queue.empty()) {
		io.set(ev::READ); // only serve if not empty
		return;
	}
	
	Buffer* buffer = response_queue.front();

	ssize_t written = write(watcher.fd, buffer->dpos(), buffer->nbytes());
	if (written < 0) {
		printf("Write error");
		return;
	}

	buffer->pos += written;
	if (buffer->nbytes() == 0) {
		response_queue.pop_front();
		delete buffer;
	}
}

void ClientHandler::read_cb(ev::io &watcher){
	char buffer[1024];
	ssize_t nread = recv(watcher.fd, buffer, sizeof(buffer), 0);
	
	if (nread < 0) {
		printf("Read error");
		return;
	}
	
	if (nread == 0) {
		// suicide... this is not a good design actually
		//delete this;
		HttpServer::manager.remove(client_sock);
	} else {
		string strreq(buffer, nread);
		string url = get_url(strreq);
		printf("GET %s\n", url.c_str());
		string response = GetResponse(HttpServer::webdir, url);
		
		response_queue.push_back(new Buffer(response.c_str(), response.length()));
	}
}

int ClientHandler::numclient = 0;

//----------

HandlerManager::HandlerManager(){}

HandlerManager::~HandlerManager(){
	removeAll();
}

void HandlerManager::add(int sock){
	ClientHandler *client = new ClientHandler(sock);
	handlers.insert(make_pair(sock, client));
}

void HandlerManager::remove(int sock){
	map<int,ClientHandler*>::iterator it = handlers.find(sock);
	
	if (it != handlers.end()){
		delete it->second;
		handlers.erase(it);
	}
}

void HandlerManager::removeAll(){
	map<int,ClientHandler*>::iterator it = handlers.begin();
	
	while (it != handlers.end()) {
		delete it->second;
		handlers.erase(it++);
	}
}

//-----


string HttpServer::webdir;
HandlerManager HttpServer::manager;

HttpServer::HttpServer(string wdir, int port){
	init_http_parser();
	
	webdir = wdir;
	
	printf("Server starts on port %d\n", port);
	struct sockaddr_in server_addr;
	
	// build and bind socket
	
	sock= socket(PF_INET, SOCK_STREAM, 0);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
	
	int yes =1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		printf("setsockopt");
		exit(1);
	}
	
	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		printf("Binding error\n");
		exit(1);
	}
	
	// add non blocking flag to socket
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK); 
	listen(sock, 5);
	
	//set io with callback io_accept
	io.set<HttpServer, &HttpServer::io_accept>(this);
	io.start(sock, ev::READ); // wait for read event on socket
	
	// set sio with callback signal_cb
	sio.set<&HttpServer::signal_cb>();
	sio.start(SIGINT); // wait for interrupt signal
}

HttpServer::~HttpServer(){
	shutdown(sock, SHUT_RDWR);
	close(sock);
	
	printf("Closing server\n");
	
	manager.removeAll();
}

void HttpServer::io_accept(ev::io &watcher, int revents){
	if (EV_ERROR & revents) {
		printf("Error: Invalid event");
		return;
	}
	
	// try accept client connection
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	int client_sock = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);
	
	if (client_sock < 0) {
		printf("Error: can't accept client");
		return;
	}

	// handle client
	manager.add(client_sock);
}

void HttpServer::signal_cb(ev::sig &signal, int revents){
	signal.loop.break_loop(); // stop main loop
}

