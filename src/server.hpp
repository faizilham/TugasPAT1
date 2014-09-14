#ifndef SERVER_HPP
#define SERVER_HPP

#include <unistd.h>
#include <fcntl.h>
#include <ev++.h>
#include <cstdlib>

#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <cstring>
#include <list>

#include "httpparser.hpp"

struct Buffer {
	char *data;
	ssize_t len;
	ssize_t pos;

	Buffer(const char *bytes, ssize_t nbytes) {
		pos = 0;
		len = nbytes;
		data = new char[nbytes];
		memcpy(data, bytes, nbytes);
	}

	virtual ~Buffer() {
		delete [] data;
	}

	char *dpos() {
		return data + pos;
	}

	ssize_t nbytes() {
		return len - pos;
	}
};

class ClientHandler{
	private:
		ev::io io;
		std::list<Buffer*> response_queue;
		
		static int numclient;
		int client_sock;
		void callback(ev::io &watcher, int revents);
		void write_cb(ev::io &watcher);
		void read_cb(ev::io &watcher);
		
	public:
		ClientHandler (int sock);
		virtual ~ClientHandler();
		
};

class HandlerManager{
	private:
		map<int, ClientHandler*> handlers;
	public:
		HandlerManager();
		virtual ~HandlerManager();
		void add(int sock);
		void remove(int sock);
		
		void removeAll();
};

class HttpServer {
	private:
		ev::io io; // io event
		ev::sig sio; // signal event
		int sock;
		
	public:
		static string webdir;
		static HandlerManager manager;
		HttpServer(string wdir, int port);
		virtual ~HttpServer();
		void io_accept(ev::io &watcher, int revents);
		static void signal_cb(ev::sig &signal, int revents);
		
};
#endif