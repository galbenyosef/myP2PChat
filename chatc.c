#include "chat_shared.h"

chat_server server;
chat_client self;
chat_peer connection;
chat_peer peer;

void init_self();
void set_server_address();
void connect_to_server();
void connect_to_();
void disconnect_from_peer();
void respond_to_client(int val);
void set_initer(chat_peer* connection,in_port_t port,char* name);
void set_accepter(chat_peer* connection);
msg_text_t* prepare_text_msg(int buff_size);
void send_end_msg();
void send_text_msg(char* buffer,int buff_size);
void* terminator(void* arg);
void* receiver(void* arg);
void* connection_receive(void* arg);
void* receiver(void* arg);
void* host_thread(void* arg);
void host();
void who_respond_handler();
void who();
int hdr_respond();
void up();
void down();
void up_respond();
void msg_conn();
void msg_conn_respond_handler();
void input_handler();
void ack_handler();
void nack_handler();


int main(int argc, char** argv) {

    msg_type_t respond;
    init_self();
    set_server_address();
    connect_to_server();
    up();
    up_respond(&respond);
    
    switch (respond){
        case (MSG_ACK):{
            ack_handler();
            break;
        }
        case (MSG_NACK):{
            nack_handler();
            return 1;
        }
        default:{
            return 1;
        }
    }

    return (EXIT_SUCCESS);
}


void init_self(){
    
    
    if ((server.socket = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        write(STDERR_FILENO,"Error: server socket\n",sizeof("Error: server socket\n"));
        exit(EXIT_FAILURE);
    }
   
    if ((self.socket = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        write(STDERR_FILENO,"Error: self socket\n",sizeof("Error: self socket\n"));
        exit(EXIT_FAILURE);
    }
    
    char name[C_NAME_LEN+1];
    write(STDOUT_FILENO,"Hello guest, please enter your name:\n",sizeof("Hello guest, please enter your name:\n"));
    int name_length = my_reader(name,C_NAME_LEN);
    
    strncpy(self.name,name,name_length);
    self.name[name_length]='\0';
    
    inet_aton(LOCALHOST,&self.address.sin_addr);
    self.address.sin_addr.s_addr = INADDR_ANY;
    
}

void set_server_address(){

    server.address.sin_family = AF_INET;
    inet_aton(LOCALHOST,&server.address.sin_addr);
    server.address.sin_port = htons(C_SRV_PORT);
    
}

void connect_to_server(){ //should occur only once
    
    
    if (connect(server.socket,(struct sockaddr*)&server.address,sizeof(struct sockaddr_in)) < 0){
        write(STDERR_FILENO,"Error: self connect\n",sizeof("Error: self connect\n"));
        exit(EXIT_FAILURE);
    }
    server.is_active=true;
    
}

void connect_to_(chat_peer* connection){
    
    if ((connection->socket_out = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        write(STDERR_FILENO,"Error: peer.socket_in\n",sizeof("Error: peer.socket_in\n"));
        exit(EXIT_FAILURE);
    }
    
    connection->address.sin_family=AF_INET;
    inet_aton(LOCALHOST,&(peer.address.sin_addr));
    
    if (connect(connection->socket_out,(struct sockaddr*)&(connection->address),sizeof(struct sockaddr_in)) < 0){
        write(STDERR_FILENO,"Error: peer connect\n",sizeof("Error: peer connect\n"));
        exit(EXIT_FAILURE);
    }
    
}

void disconnect_from_peer(){

    write(STDOUT_FILENO,"Chat session ended.\n",sizeof("Chat session ended.\n"));
    self.connected=false;
    peer.address.sin_port = 0;
    strncpy(peer.name,"",sizeof(""));
        
}

void respond_to_client(int val){
    
    msg_resp_t rsp;
    rsp.m_type = MSG_RESP;
    rsp.m_agree = val;
    
    if (send(connection.socket_in, &rsp, sizeof(msg_resp_t), 0) < 0) {
        write(STDERR_FILENO, "Error: respond_to_peer\n", sizeof("Error: respond_to_peer\n"));
        exit(EXIT_FAILURE);
    }

}

void set_initer(chat_peer* connection,in_port_t port,char* name){
    
    connection->address.sin_port = port;
    strncpy(connection->name,name,strlen(name));

}

void set_accepter(chat_peer* connection){ //reads desired p2p client's port in order to communicate
    
    char port_num[5];
    write(STDOUT_FILENO,"Enter port number:\n",sizeof("Enter port number:\n"));
    my_reader(port_num,5);
    int port = atoi(port_num);

    connection->address.sin_port=htons(port);
    
}

msg_text_t* prepare_text_msg(int buff_size){
    
    msg_text_t* text;
    text = (msg_text_t*)malloc(sizeof(msg_text_t)+buff_size);
    text->m_size= buff_size;
    return text;
    
}

void send_end_msg(){
    msg_end_t end;
    end.m_type = MSG_END;

    if (send(peer.socket_out, &end, sizeof(msg_end_t), 0) < 0) {
        write(STDERR_FILENO, "Error: send msg_end_t\n", sizeof("Error: send msg_end_t\n"));
        exit(EXIT_FAILURE);
    }
}

void send_text_msg(char* buffer,int buff_size){

    msg_text_t* text = prepare_text_msg(buff_size);
    //send msg_end_t
    if (!strcmp(buffer,"end") || !strcmp(buffer,"exit")){
        send_end_msg();
        disconnect_from_peer();
        free(text);
        return;
    }
    //send msg_text_t
    else{
        strncpy(text->m_text, buffer, buff_size);
        text->m_type = MSG_TEXT;
        if (send(peer.socket_out, text, sizeof(msg_text_t)+buff_size, 0) < 0) {
            write(STDERR_FILENO, "Error: msg_text_t\n", sizeof("Error: msg_text_t\n"));
            exit(EXIT_FAILURE);
        }
        free(text);
    }

}

void* terminator(void* arg){
    
    
    while(1){
        msg_type_t respond;
        if (recv(server.socket,&respond,sizeof(msg_type_t),MSG_PEEK) < 0){
            write(STDERR_FILENO,"Error: terminator\n",sizeof("Error: terminator\n"));
        }
        if (recv(server.socket,&respond,sizeof(msg_type_t),MSG_PEEK) == 0){
            if (recv(server.socket,&respond,sizeof(msg_shutdown_t),MSG_WAITALL) < 0){
                write(STDERR_FILENO,"Error: respond msg_who_t\n",sizeof("Error: respond msg_who_t\n"));
            }
            if(self.connected)
                disconnect_from_peer();
            down();
            server.is_active=false;
            break;
        }
    }
    
    return NULL;
    
}

void* receiver(void* arg){ //called once by each client after receiving ack from msg_up and is ready to listen
    write(STDOUT_FILENO, "Now on chat session...\n", sizeof("Now on chat session...\n"));
    while (1){

	msg_type_t msg;
        
        if (recv(peer.socket_in, &msg, sizeof(msg_type_t), MSG_PEEK) < 0) {
            write(STDERR_FILENO, "Error: receiver\n", sizeof("Error: receiver\n"));
                return NULL;
        }
        if (recv(peer.socket_in, &msg, sizeof(msg_type_t), MSG_PEEK) == 0) {
                return NULL;
        }
        switch (msg) {

            case (MSG_TEXT): {
                msg_text_t peek_content;
                msg_text_t* content;
                int text_length;

                if (recv(peer.socket_in, &peek_content, sizeof(msg_text_t), MSG_PEEK) < 0) {
                        write(STDERR_FILENO, "Error: recv msg_who_t\n", sizeof("Error: respond msg_who_t\n"));
                        return NULL;
                }

                text_length = peek_content.m_size;
                content = (msg_text_t*)malloc(sizeof(msg_text_t) + sizeof(char)*text_length);

                if (recv(peer.socket_in, content, sizeof(msg_text_t)+sizeof(char)*text_length, MSG_WAITALL) < 0) {
                        write(STDERR_FILENO, "Error: recv msg_who_t\n", sizeof("Error: respond msg_who_t\n"));
                        return NULL;
                }
                write(STDOUT_FILENO, content->m_text, strlen(content->m_text));
                write(STDOUT_FILENO, "\n", sizeof("\n"));
                free(content);
                break;
            }
            case (MSG_END): {
                msg_end_t content;
                if (recv(peer.socket_in, &content, sizeof(msg_end_t), MSG_WAITALL) < 0) {
                        write(STDERR_FILENO, "Error: recv msg_who_t\n", sizeof("Error: respond msg_who_t\n"));
                        return NULL;
                }
                disconnect_from_peer();
                return NULL;
            }
        }
    }
}

void* connection_receive(void* arg){
    
    while (1){

	msg_type_t msg;


        if (recv(connection.socket_in, &msg, sizeof(msg_type_t), MSG_PEEK) < 0) {
                write(STDERR_FILENO, "Error: recv connection_receive\n", sizeof("Error: recv connection_receive\n"));
                return NULL;
        }
        if (recv(connection.socket_in, &msg, sizeof(msg_type_t), MSG_PEEK) == 0) {
                return NULL;
        }

        switch (msg) {
            msg_conn_t content;
            case (MSG_CONN): {
                if (recv(connection.socket_in, &content, sizeof(msg_conn_t), MSG_WAITALL) < 0) {
                        write(STDERR_FILENO, "Error: recv msg_conn_t\n", sizeof("Error: respond msg_conn_t\n"));
                        return NULL;
                }
                if (!self.connected){
                    pthread_t receiver_thread;
                    peer.socket_in = connection.socket_in;
                    pthread_create(&receiver_thread,NULL,receiver,NULL);
                    set_initer(&peer,content.m_port,content.m_name);
                    respond_to_client(1);
                    connect_to_(&peer);
                    self.connected=true;
                    
                    msg_type_t chat_acceptance = 99;
                    send(peer.socket_out,&chat_acceptance,sizeof(msg_type_t),0);
                }
                else{
                    respond_to_client(0);

                }                
                break;
            }
            case (99):{
                if (recv(connection.socket_in, &content, sizeof(msg_type_t), MSG_WAITALL) < 0) {
                    write(STDERR_FILENO, "Error: recv msg_conn_t\n", sizeof("Error: respond msg_conn_t\n"));
                    return NULL;
                }
                pthread_t receiver_thread;
                peer.socket_in=connection.socket_in;
                pthread_create(&receiver_thread,NULL,receiver,NULL);
                
            }
        }
    }
}


void* host_thread(void* arg){ //waiting for acceptence then signaling
    

    if ((bind(self.socket,(struct sockaddr*)&self.address, sizeof(struct sockaddr_in))) < 0) {
        write(STDERR_FILENO,"Error: self bind\n",sizeof("Error: self bind\n"));
        return NULL;
    }

    listen(self.socket,5);

    while(1){
        
        pthread_t connection_thread;
        connection.socket_in = accept(self.socket,NULL,0);
        pthread_create(&connection_thread,NULL,connection_receive,NULL);
    }

}

void host(){ //calls host_thread
    
    pthread_t thread;
    if (pthread_create(&thread,NULL,host_thread,NULL) < 0){
        write(STDERR_FILENO,"Error: host pthread_create\n",sizeof("Error: host pthread_create\n"));
        return;
    }
    
}

void who(){ //sends who msg to server.socket

    msg_who_t msg;
    msg.m_type = MSG_WHO;
    if (send(server.socket,&msg,sizeof(msg_who_t),0) < 0 ){
        write(STDERR_FILENO,"Error: send msg_who_t\n",sizeof("Error: send msg_who_t\n"));
        return;    
    }
    
}

int hdr_respond(){
    
    msg_type_t resp;
    if (recv(server.socket,&resp,sizeof(msg_hdr_t),MSG_PEEK) < 0){
        write(STDERR_FILENO,"Error: hdr_respond\n",sizeof("Error: hdr_respond\n"));
    }
    if (resp == MSG_HDR){
        msg_hdr_t hdr;
        if (recv(server.socket,&hdr,sizeof(msg_hdr_t),MSG_WAITALL) < 0){
            write(STDERR_FILENO,"Error: hdr_respond\n",sizeof("Error: hdr_respond\n"));
        } 
        return hdr.m_count;
    }
}

void who_respond_handler(){

    int count = hdr_respond();
    
    if (count>0){
        write(STDOUT_FILENO,"Listing users:\n",sizeof("Listing users:\n"));
    }
    int i=0;
    for (;i<count;i++){
        msg_type_t respond;
        if (recv(server.socket,&respond,sizeof(msg_type_t),MSG_PEEK) < 0){
            write(STDERR_FILENO,"Error: respond msg_who_t\n",sizeof("Error: respond msg_who_t\n"));
            return;  
        }
        if (respond == MSG_PEER){
            msg_peer_t peer;
            if (recv(server.socket,&peer,sizeof(msg_peer_t),MSG_WAITALL) < 0){
                write(STDERR_FILENO,"Error: respond msg_who_t\n",sizeof("Error: respond msg_who_t\n"));
                return;  
            }

            write(STDOUT_FILENO,peer.m_name,strlen(peer.m_name));
            write(STDOUT_FILENO," ",sizeof(" "));
            int c=strlen(peer.m_name);
            while(C_NAME_LEN-c>0){
                write(STDOUT_FILENO,".",sizeof("."));
                ++c;
            }
            char tmp[12];
            sprintf(tmp,"%11d",peer.m_port);
            write(STDOUT_FILENO,&tmp,sizeof(tmp));
            write(STDOUT_FILENO,"\n",sizeof("\n"));
        }
    }
}

void up(){ //send up_msg to server.socket
    
    msg_up_t msg;
    msg.m_type = MSG_UP;
    inet_aton(LOCALHOST,(struct in_addr*)&msg.m_addr);
    strncpy(msg.m_name,self.name,C_NAME_LEN+1);
    if (send(server.socket,&msg,sizeof(msg_up_t),0) < 0 ){
        
        write(STDERR_FILENO,"Error: self msg_up_t\n",sizeof("Error: self msg_up_t\n"));
        return;
        
    }
    
    pthread_create(&self.terminator,NULL,terminator,NULL);
}

void down(){ //send down msg to server.socket
    
    msg_down_t msg;
    msg.m_type = MSG_DOWN;
    inet_aton(LOCALHOST,(struct in_addr*)&msg.m_addr);
    msg.m_port = htons(self.address.sin_port);

    if (send(server.socket,&msg,sizeof(msg_down_t),0) < 0 ){
        
        write(STDERR_FILENO,"Error: msg_down_t\n",sizeof("Error: msg_down_t\n"));
        return;
        
    }
    close(peer.socket_out);
    close(peer.socket_in);
    close(self.socket);
}

void up_respond(msg_type_t* respond){ //receives up_msg respond from server.socket and returns it
    

    if (recv(server.socket,respond,sizeof(msg_type_t),MSG_PEEK) < 0 ){
        
        write(STDERR_FILENO,"Error: msg_up_t respond\n",sizeof("Error: msg_up_t respond\n"));
        *respond = MSG_ERR;
        
    }
}

void msg_conn(){ //sends msg_conn_t to the desired p2p client
    
    msg_conn_t msg;
    msg.m_type=MSG_CONN;
    inet_aton(LOCALHOST,(struct in_addr*)&msg.m_addr);
    strncpy(msg.m_name,self.name,strlen(self.name));
    msg.m_port=self.address.sin_port;
    
    peer.socket_out=connection.socket_out;
    
    if (send(connection.socket_out,&msg,sizeof(msg_conn_t),0) < 0 ){
        write(STDERR_FILENO,"Error: self msg_conn_t\n",sizeof("Error: self msg_conn_t\n"));
        return;
    }
    
}

void msg_conn_respond_handler() { //receievs msg_conn_t respond from the desired p2p client and returns it
    
    msg_type_t respond;
    
    if (recv(connection.socket_out,&respond,sizeof(msg_type_t),MSG_PEEK) < 0 ){
        write(STDERR_FILENO,"Error: self msg_conn_t\n",sizeof("Error: self msg_conn_t\n"));
        respond = MSG_ERR;
    }
    
    switch (respond){
        case (MSG_RESP):{
            msg_resp_t resp;
            if (recv(connection.socket_out,&resp,sizeof(msg_resp_t),MSG_WAITALL) < 0 ){
                write(STDERR_FILENO,"Error: respond msg_resp_t\n",sizeof("Error: respond msg_resp_t\n"));
                return;
            }
            if (resp.m_agree == 1){
                    
                self.connected=true;
            }
            else{
                write(STDOUT_FILENO,"Peer is currently busy...\n",sizeof("Peer is currently busy...\n"));
            }
        }
    }
}




void input_handler(){ //server interface, called when client received msg_up_t ack
    
       
    write(STDOUT_FILENO,"You are now allowed to send commands\n",sizeof("You are now allowed to send commands\n"));
    write(STDOUT_FILENO,"Help Menu:\n",sizeof("Help Menu:\n"));
    write(STDOUT_FILENO,"'w' - who:\n",sizeof("'w' - who:\n"));
    write(STDOUT_FILENO,"'c' - connect:\n",sizeof("'c' - connect:\n"));
    write(STDOUT_FILENO,"'x' - exit:\n",sizeof("'x' - exit:\n"));

    while (1){

        char input[C_BUFF_SIZE];
        int input_size = my_reader(input,C_BUFF_SIZE);

        if (server.is_active==false){
            exit(EXIT_SUCCESS);
        }
        if (self.connected){
            send_text_msg(input,input_size);
        }
        else{
        
            switch (input[0]){
                
                case ('w'):{

                    who();
                    who_respond_handler();
                    
                    break;
                }
                case ('c'):{

                    set_accepter(&connection);
                    connect_to_(&connection);
                    msg_conn();
                    msg_conn_respond_handler();
                    
                    break;
                }
                case ('x'):{
                    
                    down();
                    return;
                    
                }
                default:{
                    break;
                }
            }
        }
    }
}

void ack_handler(){
    msg_ack_t msg;
    if (recv(server.socket,&msg,sizeof(msg_ack_t),MSG_WAITALL) < 0 ){
        write(STDERR_FILENO,"Error: msg_ack_t respond\n",sizeof("Error: msg_ack_t respond\n"));
        return;
    }
    self.address.sin_port=htons(msg.m_port);
    host();
    input_handler();
}

void nack_handler(){
    msg_nack_t msg;
    if (recv(server.socket,&msg,sizeof(msg_nack_t),MSG_WAITALL) < 0 ){
        write(STDERR_FILENO,"Error: msg_nack_t respond\n",sizeof("Error: msg_nack_t respond\n"));
        return;
    }
    write(STDERR_FILENO,"Connection rejected\n",sizeof("Connection rejected\n"));
}
