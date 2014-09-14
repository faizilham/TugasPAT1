#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

/*
* Very basic http parser
*/

#include <map>
#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>

using namespace std;

static inline string itostr(int i){
	char buf[10];
	sprintf(buf, "%d", i);
	return string(buf);
}

static map<int, string> status_desc;
static string newline = "\r\n";
static string header_end = newline + newline;

static inline void init_http_parser(){
	status_desc[200] = "200 OK";
	status_desc[404] = "404 Not Found";
}

static inline string build_response(int status, string type, string message){
	string statusline = "HTTP/1.1 " + status_desc[status];
	string entityheader = "Content-Type: " + type + newline + "Content-Length: " + itostr(message.length());
	
	return statusline + newline + entityheader + header_end + message;
}

static inline map<string,string> get_request(string request){
	map<string,string> reqmap;

	int space1 = request.find(" ");
	int space2 = request.find(" ", space1+1);
	
	int urllen = space2 - space1 - 1;
	
	reqmap["url"] = request.substr(space1 + 1, urllen);
	reqmap["method"] = request.substr(0, space1);
	
	return reqmap;
}

static inline string get_url(string request){
	map<string,string> reqmap;

	int space1 = request.find(" ");
	int space2 = request.find(" ", space1+1);
	
	int urllen = space2 - space1 - 1;
	
	return request.substr(space1 + 1, urllen);
}

static string file404 = "<html><head></head><body><h2>Error 404: File Not Found</h2></body></html>";

static inline string GetResponse(string webdir, string URL){
	ifstream inFile;
	string filePath, response;
	
	// Send HTML Response
	filePath = webdir + URL;
	
	// If path ends with '/', requesting index
	if(filePath.at( filePath.length() - 1 ) == '/'){
		filePath += "index.html";
	}
	
	inFile.open(filePath.c_str());
	if(!inFile){
		response = build_response(404, "text/html", file404);
		return response;
	}else{
		string output;
		stringstream stream;
		
		stream << inFile.rdbuf();
		output = stream.str();
		
		response = build_response(200, "text/html", output);
		return response;
		inFile.close();
	}
}

#endif