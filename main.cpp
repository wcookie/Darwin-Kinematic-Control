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



//using namespace webots;
static easywsclient::WebSocket::pointer ws = NULL;
static Walk *controller = new Walk();

// "WALK 5.8 30.0"
// walk --> Walk(5,8 30.0);
// "STOPWALK"
// stopWalking();



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

    ws = easywsclient::WebSocket::from_url("ws://localhost:8126/foo");
    assert(ws);

    ws->send("hello");

    while (ws->getReadyState() != easywsclient::WebSocket::CLOSED) {
        ws->poll();
        ws->dispatch(handle_message);
    }   
    delete ws; 
#ifdef _WIN32
    WSACleanup();
#endif
    delete controller;
    return EXIT_FAILURE;
}
