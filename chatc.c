//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include "chat.h"
//#include <string.h>
//
//int main(int argc, char** argv) {
//
//    int sockfd;
//    msg_up_t upMsg;
//    char server_reply[2048];
//    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
//        perror("client to server socket");
//    }
//    struct sockaddr_in servaddr;
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_port = htons(C_SRV_PORT);
//    inet_aton("127.0.0.1", &servaddr.sin_addr);
//
//    
//    upMsg.m_addr = servaddr.sin_addr.s_addr; // change to client address struct(same address)
//    upMsg.m_type = MSG_UP;
//    char* name = "Matan";
//    strcpy(upMsg.m_name ,name);
//    
//    if ((connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
//        perror("connect");
//    }
//    
//    if (send(sockfd,(void*)&upMsg,11,0)<0){
//        perror("send");
//    }
//    
//    if (recv(sockfd,server_reply,2000,0)<0){
//        perror("recv failed");
//    }
//    
//    
//    return (EXIT_SUCCESS);
//}
//
