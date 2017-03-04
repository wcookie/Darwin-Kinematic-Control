//websocket
#include "easywsclient.hpp" // <-- include only if you don't want compile separately
//#include "easywsclient.cpp"
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include "Walk.hpp"
//webots
#include "walk-client.h"
#include <cstdlib>
#include <vector>
#include "webots-client.h"


//using namespace webots;

static easywsclient::WebSocket::pointer ws = NULL;

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
    while (curr2 < r.length()){
        if (r[curr2] != ' '){
            c2 += r[curr2];
            curr2++;
        } else{
            speed = stod(c2);
            r2 = r.substr(curr2 + 1, r.length());
            angle = stod(r2);
            move(speed, angle);
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
                stopMov();
                ws->close();
                break;
            }
        }
    }
}



int main()
{

#ifdef _WIN32
    INT rc; 
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
    return 0;
}
