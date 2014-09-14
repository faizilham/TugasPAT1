#include "server.hpp"

int main(){
	ifstream inFile; string str; string webdir;
	inFile.open("config");
	
	int port;
	
	if(!inFile){
		printf("Config file not found");
		return 1;
	}else{
		getline(inFile, webdir);
		getline(inFile, str);
		
		port = atoi(str.c_str());
		
		inFile.close();
	}
	
	printf("Serving directory %s\n", webdir.c_str());
	
	ev::default_loop loop;
	HttpServer server(webdir, port);
  
    loop.run(0);
  
    return 0;
}