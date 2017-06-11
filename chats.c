#include "chat.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include  <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>


//Global variables//
in_port_t give_port = 12361;
int shutdown_ = 0; //shutdown flag 

/*implementation of the servers thread*/
void* thread_implem(void * socket); 

int main(int argc, char** argv) {

    struct sockaddr_in server, client;
    int serv_socket, client_sock, sizeof_addrin;

    //Create socket with given port num
    serv_socket = socket(AF_INET, SOCK_STREAM, 0); //create a server socket
    if (serv_socket < 0) {
            perror("Failed to create socket");
    }
    //Create sockaddr_in struct
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(C_SRV_PORT);

    if (bind(serv_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
            perror("binding server failed");
            return 1;
    }
    listen(serv_socket, 3); //start receiving upcoming connections from sockets
    //Accept and incoming connection
    puts("IM ALIVE");
    sizeof_addrin = sizeof(struct sockaddr_in);

//connections get accepted here untill shutdown is initiated
    while ((client_sock = accept(serv_socket, (struct sockaddr *) &client,(socklen_t*) &sizeof_addrin))) {
        puts("Connection accepted");

        pthread_t t; //thread for the accepted request
        int sock;
        sock = client_sock;

        /*handles shutdown initiation*/
        if (shutdown_ == 1) {
                pthread_exit(0);
                shutdown(serv_socket,SHUT_RDWR );
                return 0;
        }
        //create thread and run the thread implementation function
        if (pthread_create(&t, NULL, thread_implem, (void*) &sock) != 0) {
                perror("could not create thread");
                return 1;
        }
        /*handles shutdown initiation*/
        if (shutdown_ == 1) {
                pthread_exit(0);
                shutdown(serv_socket,SHUT_RDWR );
                return 0;
        }
    }
    return 0;
}


/*thread_implementation handles the messages by type*/
void* thread_implem(void * socket) {
    
    int sock = *(int*) socket;
    int typ, in_msg;
    msg_ack_t ak;

    /*cuntinue while msgs are comming from this connection*/
    while ((in_msg = recv(sock, (void*)&typ, sizeof(int), MSG_PEEK)) > 0) {
        msg_type_t theType = ((msg_ack_t*)typ)->m_type;
        int swtch = (int)theType;
        /*swich case by msg type*/
        switch (swtch) {

        case MSG_UP: {
            msg_peer_t new_peer;
            recv(sock, (void*) &new_peer, sizeof(msg_peer_t), 0);
            /*server answer to client*/	
            ak.m_port = htons(give_port); 
            ak.m_type = MSG_ACK;
            send(sock, (void*) &ak, sizeof(msg_ack_t), 0); //after appending new msg sent ok (ACK)
            give_port++;		
            break;	
        }
        case MSG_DOWN: {

        }
        case MSG_WHO: {

        }
        case MSG_SHUTDOWN: { //handle server stop signal from "remote" connection
            fprintf(stdout, "disconnecting server... I'LL BE BACK!! \n");
            shutdown_ = 1;
            fflush(stdout);
            shutdown(sock,SHUT_RDWR );
            pthread_exit(0);
        }
        default:
            break;
        }
    }
    return 0;
}






