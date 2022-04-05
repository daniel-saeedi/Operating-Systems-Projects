#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

// Declaration of enum
#define True 1
#define False 0
typedef enum { F, T } boolean;

#define PORT_BASE 9090

// Linked List
struct ClientQueue {
    int clinet_id;
    struct ClientQueue* next;
};

struct Player{
    int client_id;
};

int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

boolean is_user_in_queue(struct ClientQueue* client_head, int client_id)
{
    struct ClientQueue* current_node = client_head;
    while(current_node != NULL)
    {
        if (current_node->clinet_id == client_id) return True;
        current_node = current_node->next;
    }

    return False;
}


// This function adds new client who is ready for a game to ClientQueue
struct ClientQueue* handle_ready(struct ClientQueue* client_head,int client_id)
{
    struct ClientQueue* last_node = NULL;
    struct ClientQueue* current_node = client_head;
    while(current_node != NULL)
    {
        if(current_node->next == NULL){
            last_node = current_node;
            break;
        }
        else
            current_node = current_node->next;
    }

    if(is_user_in_queue(client_head,client_id) == True)
    {
        printf("User already in queue!\n");
        return client_head;
    }

    //Make head
    if(last_node == NULL)
    {
        client_head = (struct ClientQueue*)malloc(sizeof(struct ClientQueue));
        client_head->clinet_id = client_id;
        client_head->next = NULL;
    }
    // Add new node
    else{
        struct ClientQueue* new_node = (struct ClientQueue*)malloc(sizeof(struct ClientQueue));
        new_node->clinet_id = client_id;
        new_node->next = NULL;
        last_node->next = new_node;
    }

    return client_head;
}

char* convert_int_to_str(int x){
    int length = snprintf( NULL, 0, "%d", x );
    char* x_string = malloc( length + 1 );
    snprintf( x_string, length + 1, "%d", x );

    return x_string;
}

void create_new_game(struct Player* player_ready1,struct Player* player_ready2,int port)
{
    char* port_string = convert_int_to_str(port);
    char* clientid1_string = convert_int_to_str(player_ready1->client_id);
    char* clientid2_string = convert_int_to_str(player_ready2->client_id);
    
    char player1_msg[2000];
    strcpy(player1_msg,port_string);
    strcat(player1_msg, " ");
    strcat(player1_msg, clientid1_string);
    strcat(player1_msg, " ");
    strcat(player1_msg, clientid2_string);
    strcat(player1_msg, " ");
    strcat(player1_msg, "1");
    write(player_ready1->client_id, player1_msg, sizeof(player1_msg));
    
    char player2_msg[2000];
    strcpy(player2_msg,port_string);
    strcat(player2_msg, " ");
    strcat(player2_msg, clientid2_string);
    strcat(player2_msg, " ");
    strcat(player2_msg, clientid1_string);
    strcat(player2_msg, " ");
    strcat(player2_msg, "0");
    write(player_ready2->client_id, player2_msg, sizeof(player2_msg));
    
    
}

void get_games(int client_id, int game_count){
    char game_list[1024];
    strcpy(game_list,"");
    if(game_count == -1){
        strcat(game_list,"There's no game available.\n");
    }else{
        for(int i = 0; i <= game_count;i++){
            int game_port = PORT_BASE + i;
            int length = snprintf( NULL, 0, "%d", game_port );
            char* str = malloc( length + 1 );
            snprintf( str, length + 1, "%d", game_port );
            strcat(game_list,str);
            strcat(game_list,"\n");
        }
    }
    write(client_id, game_list, sizeof(game_list));
}


void run(int port){
    int server_fd, new_socket, max_sd;
    char buffer[2000] = {0};
    fd_set master_set, working_set;
       
    server_fd = setupServer(port);

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    

    write(1, "Server is running\n", 18);

    //
    // struct ClientQueue* client_head = NULL;

    int game_count = -1;
    struct Player* player_ready1 = NULL;
    struct Player* player_ready2 = NULL;

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == server_fd) {  // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                }
                
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, 2000, 0);
                    
                    if (bytes_received == 0) { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    char original_buffer[2000] = {0};
                    strcpy(original_buffer,buffer);
                    // Remove new line
                    buffer[strcspn(buffer, "\n")] = 0;

                    // Ready Command
                    if(strcmp(buffer,"ready") == 0){
                        if(player_ready1 != NULL){
                            if(player_ready1->client_id != i){
                                player_ready2 = (struct Player*)malloc(sizeof(struct Player));
                                player_ready2->client_id = i;

                                game_count += 1;
                                create_new_game(player_ready1,player_ready2,PORT_BASE+game_count);

                                player_ready1 = NULL;
                                player_ready2 = NULL;
                            }
                        }else{
                            player_ready1 = (struct Player*)malloc(sizeof(struct Player));
                            player_ready1->client_id = i;
                        }
                    }
                    else if(strcmp(buffer,"get_games") == 0){
                        get_games(i,game_count);
                    }
                    else{
                        // Write game result
                        printf("%s\n",original_buffer);
                        int file_fd;
                        file_fd = open("game_results.txt", O_APPEND | O_RDWR);
                        write(file_fd, original_buffer, strlen(original_buffer));
                        close(file_fd);
                    }
                    printf("client %d: %s\n", i, buffer);
                    memset(buffer, 0, 2000);
                }
            }
        }

    }
}

int main(int argc, char const *argv[]) {
    if (argc != 2)
	{
		printf("Run like this: %s <port>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	int server_port = atoi(argv[1]);
    run(server_port);
    return 0;
}
