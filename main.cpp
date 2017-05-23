//websocket
#include "easywsclient.hpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include "Walk.hpp"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>


//using namespace webots;
static easywsclient::WebSocket::pointer ws = NULL;
static Walk *controller = new Walk();

// "WALK 5.8 30.0"
// walk --> Walk(5,8 30.0);
// "STOPWALK"
// stopWalking();


void error(char *msg)
{
    perror(msg);
    exit(0);
}

void handle_walk(const std::string & r){
    size_t curr2 = 0;
    std::string c2 = ""; // the current message in this loop
    double speed = 0.0;
    double angle = 0.0;
    std::string r2;
    std::stringstream ss;
    while (curr2 < r.length()){
        if (r[curr2] != ' '){
            c2 += r[curr2];
            curr2++;
        } else{
            //speed = std::stod(c2);
            //speed = std::atoi(c2);
            ss <<c2;
            ss >>speed;
            ss.clear();
            r2 = r.substr(curr2 + 1, r.length());
            //angle = std::stod(r2);
            //angle = std::atoi(r2);
            ss << r2;
            ss >> angle;
            controller->move(speed, angle);
            break;
        }
    }

}


void handle_message(const std::string & message)
{

    size_t curr = 0; // the cursor
    std::string c (""); // the current message
    printf(">>> %s\n", message.c_str());
    std::cout<<"Handle Message \n";
    while (curr < message.length()){
        std::cout<<message[curr]<<'\n';
        if (message[curr] != ' '){
            c += message[curr];
            curr++;
        } else{
            if (c == "WALK"){
                std::string r = message.substr(curr + 1, message.length()); //remaining string
                handle_walk(r);
                break;
            }else if (c == "STOPWALK"){
                controller->stopMov();
                ws->close();
                break;
            }
        }
    }
}



int main(int argc, char **argv)
{
    controller->run();
#ifdef _WIN32
    int rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
#endif

int sockfd, portno, n;

struct sockaddr_in serv_addr;
struct hostent *server;

char buffer[256];

portno = 80;
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0)
    error("ERROR opening socket");
server = gethostbyname("10.105.183.32");
if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
}
bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
bcopy((char *)server->h_addr,
     (char *)&serv_addr.sin_addr.s_addr,
     server->h_length);
serv_addr.sin_port = htons(portno);
if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) >= 0)
{
  while(true){
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    if (buffer[0] == '*'){
      printf("exit \r\n");
      break;
    }
}
}
#ifdef _WIN32
    WSACleanup();
#endif
    delete controller;
    return EXIT_FAILURE;
}
