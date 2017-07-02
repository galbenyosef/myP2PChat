#ifndef CHAT_SHARED_H
#define CHAT_SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "chat.h"

#define S_MAX_PEERS 10
#define LOCALHOST "127.0.0.1"

typedef struct{
    
    int socket;
    struct sockaddr_in address;

    msg_peer_t active_peers[S_MAX_PEERS];
    int active_peers_counter;
    
    in_port_t port;
    
    bool is_active;
    
    pthread_t terminator;
    
}chat_server;

typedef struct{
    
    int socket; //accepting msg_conn requests
    
    struct sockaddr_in address;
    char name[C_NAME_LEN];

    pthread_t terminator;
    
    bool connected;

}chat_client;

typedef struct{
    
    
    int socket_out; //sending msg_text,msg_end requests
    int socket_in; //accepting msg_text,msg_end requests
    struct sockaddr_in address;
    char name[C_NAME_LEN];


}chat_peer;


int my_reader(char* buffer,int buffer_size){ //given buffer and maximum read characters integer, returning read count
    
    char ch;
    int i=0;
    while (read(STDIN_FILENO,&ch,1) > 0 && ch != '\n' && i < buffer_size){
        buffer[i]=ch;
        ++i;
    }
    buffer[i]='\0';
    return i+1;
    
}

#endif /* CHAT_SHARED_H */

