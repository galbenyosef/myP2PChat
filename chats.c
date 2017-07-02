#include "chat_shared.h"

chat_server server;

void killEm(){
    
    msg_shutdown_t bye;  
    bye.m_type = MSG_SHUTDOWN;
    int i = 0;
    for (;i<S_MAX_PEERS;i++){
        if (server.active_peers[i].m_port != 0){
            int tempsocket = socket(AF_INET,SOCK_STREAM,0);
            struct sockaddr_in address;
            inet_aton(LOCALHOST,&address.sin_addr);
            address.sin_port=server.active_peers[i].m_port;
            address.sin_family=AF_INET;
            sendto(tempsocket,&bye,sizeof(msg_shutdown_t),MSG_DONTROUTE,(struct sockaddr*)&address,(socklen_t)sizeof(address));
            close(tempsocket);
        }
    }
    
}

void* terminator(void* arg){
    
    char buffer[C_BUFF_SIZE];
    do{
        int buff_length = my_reader(buffer,C_BUFF_SIZE);
        buffer[buff_length]='\0';
    }
    while (strcmp(buffer,"exit")!=0);
    write(STDOUT_FILENO,"Exit key detected, exiting...\n",sizeof("Exit key detected, exiting...\n"));
    killEm();
    close(server.socket);
    server.is_active = false;
    return NULL;
}

void server_init(){
    
    if ((server.socket = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        write(STDERR_FILENO,"Error: server socket\n",sizeof("Error: server socket\n"));
        
        exit(EXIT_FAILURE);
    }
    
    server.port = C_SRV_PORT;
    
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = htonl(INADDR_ANY);
    server.address.sin_port = htons(server.port);

    if (bind(server.socket,(struct sockaddr*)&server.address, (socklen_t)sizeof(server.address)) < 0) {
        write(STDERR_FILENO,"Error: server bind\n",sizeof("Error: server bind\n"));
        exit(EXIT_FAILURE);
    }
    
    server.active_peers_counter = 0;
    
    int i = 0;
    for (;i<S_MAX_PEERS;i++){
        server.active_peers[i].m_addr = 0;
        strncpy(server.active_peers[i].m_name,"",sizeof(""));
        server.active_peers[i].m_port = 0;
        server.active_peers[i].m_type = MSG_ERR;
    }
    
    listen(server.socket,5);
    
    if (pthread_create(&server.terminator,NULL,terminator,NULL) < 0){
        write(STDERR_FILENO,"Error: server terminator\n",sizeof("Error: server terminator\n"));
        exit(EXIT_FAILURE);
    } 
    
}

void* connection_handler(void* new_socket){
    
    msg_type_t msg;
    int socket = *(int*)new_socket;
    
    while(1){

        //peek msg
        if (recv(socket,&msg,sizeof(msg_type_t),MSG_PEEK) < 0){
            write(STDERR_FILENO,"Error: server recv msg_type_t\n",sizeof("Error: server recv msg_type_t\n"));
            return NULL; 
        }
        
        if (recv(socket, &msg, sizeof(msg_type_t), MSG_PEEK) == 0) {
            write(STDOUT_FILENO,"User has disconnected.\n",sizeof("User has disconnected.\n"));
            return NULL;
	}
        
        switch (msg){
            case (MSG_UP):{
                msg_up_t content;
                //get msg
                if (recv(socket,&content,sizeof(msg_up_t),MSG_WAITALL) < 0){
                    write(STDERR_FILENO,"Error: server recv msg_up_t\n",sizeof("Error: server recv msg_up_t\n"));
                    return NULL; 
                }
                if (server.active_peers_counter < S_MAX_PEERS){
                    //giving this port
                    ++server.port;
                    int i = 0;
                    for (;i<S_MAX_PEERS;i++){
                        //look for empty space
                        if (server.active_peers[i].m_port == 0){
                            server.active_peers[i].m_type = MSG_PEER;
                            server.active_peers[i].m_addr = content.m_addr;
                            server.active_peers[i].m_port = server.port;
                            strncpy(server.active_peers[i].m_name,content.m_name,strlen(content.m_name));
                            break;
                        }
                    }
                    write(STDOUT_FILENO,content.m_name,strlen(content.m_name));
                    write(STDOUT_FILENO," is now authorized",sizeof(" is now authorized"));
                    write(STDOUT_FILENO,"\n",sizeof("\n"));
                    msg_ack_t ack;
                    ack.m_type = MSG_ACK;
                    ack.m_port = server.port;
                    
                    ++server.active_peers_counter;    
                    //ack
                    if (send(socket,&ack,sizeof(msg_ack_t),0) < 0){
                        write(STDERR_FILENO,"Error: server send msg_ack_t\n",sizeof("Error: server send msg_ack_t\n"));
                        return NULL;
                    }
                }
                else{
                    msg_nack_t nack;
                    nack.m_type = MSG_NACK;
                    //nack
                    if (send(socket,&nack,sizeof(msg_nack_t),0) < 0){
                        write(STDERR_FILENO,"Error: server send msg_nack_t\n",sizeof("Error: server send msg_nack_t\n"));
                        return NULL;
                    }
                }
                break;
            }
            case (MSG_DOWN):{
                msg_down_t content;
                //get msg
                if (recv(socket,&content,sizeof(msg_down_t),MSG_WAITALL) < 0){
                    write(STDERR_FILENO,"Error: server recv msg_down_t\n",sizeof("Error: server recv msg_down_t\n"));
                    return NULL; 
                }
                int i = 0;
                //look for desired client to kick of the list
                for (;i<S_MAX_PEERS;i++){
                    if (server.active_peers[i].m_port == content.m_port){
                        server.active_peers[i].m_addr = 0;
                        strncpy(server.active_peers[i].m_name,"",sizeof(""));
                        server.active_peers[i].m_port = 0;
                        server.active_peers[i].m_type = MSG_ERR;
                    }
                }
                --server.active_peers_counter;
                
                break;
            }
            case (MSG_WHO):{
                msg_who_t content;
                if (recv(socket,&content,sizeof(msg_who_t),MSG_WAITALL) < 0){
                           write(STDERR_FILENO,"Error: server recv msg_who_t\n",sizeof("Error: server recv msg_who_t\n"));
                           return NULL; 
                }
                //send hdr
                msg_hdr_t hdr;
                hdr.m_type = MSG_HDR;
                hdr.m_count = server.active_peers_counter;
                if (send(socket,&hdr,sizeof(msg_hdr_t),0) < 0){
                           write(STDERR_FILENO,"Error: server send msg_who_t\n",sizeof("Error: server send msg_who_t\n"));
                           return NULL; 
                }
                //send all peers
                int i = 0;
                for (;i<S_MAX_PEERS;i++){
                    if (server.active_peers[i].m_port != 0){
                        if (send(socket,&server.active_peers[i],sizeof(msg_peer_t),0) < 0){
                           write(STDERR_FILENO,"Error: server send msg_peer_t\n",sizeof("Error: server send msg_peer_t\n"));
                           return NULL; 
                        }
                    }
                }
                break;
            }
            default:{
                break;
            }   
        }
    }
}

void new_client_handler(){
    
    int new_connection;
    
    pthread_t new_connection_thread;
    
    while(server.is_active == true){
        new_connection = accept(server.socket,NULL,0);
        if (pthread_create(&new_connection_thread,NULL,connection_handler,&new_connection) < 0){
            write(STDERR_FILENO,"Error: server connection aborted\n",sizeof("Error: server connection aborted\n"));
            return;
        }
    }

}

int main(int argc, char** argv) {

    write(STDOUT_FILENO,"Server initiating...\n",sizeof("Server initiating...\n"));
    server_init();
    server.is_active = true;
    write(STDOUT_FILENO,"Server initiated!\n",sizeof("Server initiated!\n"));
    new_client_handler();
    
    
    return (EXIT_SUCCESS);
}