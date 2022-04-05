#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <math.h>
#include<signal.h>

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}


void DieWithError(char *errorMessage){
    printf("%s",errorMessage);
    exit(0);
}

char* drawBoard(char board[][3])
{
    int rows, columns;

    char* board_str =  (char*)malloc(200);
    memset(board_str, 0, 200);

    for ( rows = 0 ; rows < 3 ; rows++ )
        {
            for ( columns = 0 ; columns < 3 ; columns++ )
            {
                if(board[rows][columns]){
                    strcat(board_str,"|");
                    char *temp = (char *)malloc(10);
                    memset(temp, 0, 10);
                    temp[0] = board[rows][columns];
                    strcat(board_str,temp);
                }else{
                    strcat(board_str,"| ");
                }
            }
            strcat(board_str,"|\n");
        }
    
    return board_str;
}

void update_board(int choice,int board[3][3],char c){

    printf("Cell %d \n",choice);
    choice -= 1;
    int row = ceil(choice/3);
    int col = choice%3;
    board[row][col] = c;
}

/*
* -1: Unfinished
*  1: Client 1 won.
*  2: Client 2 won.
*  3: Tie!
*/

int check_game_status(char board[][3],int clientid_1,int clientid_2,int fd){
    //Check if the game is tie.
    int tie = 1;
    for(int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            if(board[i][j] != 'X' && board[i][j] != 'O'){
                tie = 0;
            }
        }
    }

    char server_msg[1000];
    if(tie)
    {
        printf("Tie!");
        strcpy(server_msg,drawBoard(board));
        send(fd, server_msg , strlen(server_msg), 0);
        sleep(1);
        
        return 3;
    }

    //Check if client 1 has won the game.

    if(
        // Row
        (board[0][0] == 'X' && board[0][1] == 'X' && board[0][2] == 'X' ) ||
        (board[1][0] == 'X' && board[1][1] == 'X' && board[1][2] == 'X' ) ||
        (board[2][0] == 'X' && board[2][1] == 'X' && board[2][2] == 'X' ) ||
        // Column
        (board[0][0] == 'X' && board[1][0] == 'X' && board[2][0] == 'X' ) ||
        (board[0][1] == 'X' && board[1][1] == 'X' && board[2][1] == 'X' ) ||
        (board[0][2] == 'X' && board[1][2] == 'X' && board[2][2] == 'X' ) ||
        // Diagonal
        (board[0][0] == 'X' && board[1][1] == 'X' && board[2][2] == 'X' ) ||
        (board[0][2] == 'X' && board[1][1] == 'X' && board[2][0] == 'X' )
    )
    {
        printf("Client %d has won!",clientid_1);

        strcpy(server_msg,drawBoard(board));
        send(fd, server_msg , strlen(server_msg), 0);

        // send(fd, "Finished", strlen("Finished"), 0);
        sleep(1);
        return 1;
    }
    
    //Check if client 2 has won the game.
    if(// Row
        (board[0][0] == 'O' && board[0][1] == 'O' && board[0][2] == 'O' ) ||
        (board[1][0] == 'O' && board[1][1] == 'O' && board[1][2] == 'O' ) ||
        (board[2][0] == 'O' && board[2][1] == 'O' && board[2][2] == 'O' ) ||
        // Column
        (board[0][0] == 'O' && board[1][0] == 'O' && board[2][0] == 'O' ) ||
        (board[0][1] == 'O' && board[1][1] == 'O' && board[2][1] == 'O' ) ||
        (board[0][2] == 'O' && board[1][2] == 'O' && board[2][2] == 'O' ) ||
        // Diagonal
        (board[0][0] == 'O' && board[1][1] == 'O' && board[2][2] == 'O' ) ||
        (board[0][2] == 'O' && board[1][1] == 'O' && board[2][0] == 'O' )
    )
    {
        printf("Client %d has won!",clientid_2);
        strcpy(server_msg,drawBoard(board));
        send(fd, server_msg , strlen(server_msg), 0);
        sleep(1);
        return 2;
    }
    

    return -1;
}
int is_it_my_turn;

void timeout_handler(int signum){
    if(is_it_my_turn == 0){
        is_it_my_turn = 1;
        printf("Your Turn! Your opponent didn't choose any cell!\n");

    }
    else{
        is_it_my_turn = 0;
        printf("Time out!\n");
    }

    printf("Time out!\n");
}

void serverUDP(int game_port,int clientid_1,int clientid_2,int fd,int sock,struct sockaddr_in broadcastAddr){

    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];
    int client_struct_length = sizeof(client_addr);
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    // Create UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(game_port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return;
    }

    char board[3][3]={{'1','2','3'},
                      {'4','5','6'},
                      {'7','8','9'}};

    printf("%s",drawBoard(board));
    printf("Your Turn!\n");

    // Receive client's message:
    if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
         (struct sockaddr*)&client_addr, &client_struct_length) < 0){
        printf("Couldn't receive\n");
        exit(0);
    }
    
    // Respond to client:
    strcpy(server_message, drawBoard(board));
    
    if (sendto(socket_desc, server_message, strlen(server_message), 0,
         (struct sockaddr*)&client_addr, client_struct_length) < 0){
        printf("Can't send\n");
        exit(0);
    }

    is_it_my_turn = 1;
    char buff[1024] = {0};
    signal(SIGALRM, timeout_handler);
    siginterrupt(SIGALRM, 1);
    while(1)
    {
        //Check if the game is finished
        
        if(check_game_status(board,clientid_1,clientid_2,fd) != -1){
            //Send the result to server.
            strcpy(server_message, "Finished!");
            if (sendto(socket_desc, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                printf("Can't send\n");
                exit(0);
            }

            break;
        }
        if(is_it_my_turn == 1){
            memset(buff,0,1024);
            alarm(10);
            int r = read(0, buff, 1024);
            alarm(0);
            buff[strcspn(buff, "\n")] = 0;

            if(r > 0){
                //Update the board
                int choice = atoi(buff);
                choice -= 1;
                int row = ceil(choice/3);
                int col = choice%3;
                board[row][col] = 'X';

                printf("\n%s\n",drawBoard(board));
                strcpy(server_message, drawBoard(board));

                if (sendto(socket_desc, server_message, strlen(server_message), 0,
                    (struct sockaddr*)&client_addr, client_struct_length) < 0){
                    printf("Can't send\n");
                    exit(0);
                }

                /* Broadcast sendString in datagram to clients every 3 seconds*/
                if (sendto(sock, server_message, strlen(server_message), 0, (struct sockaddr*) 
                    &broadcastAddr, sizeof(broadcastAddr)) != strlen(server_message))
                    DieWithError("sendto() sent a different number of bytes than expected");

                is_it_my_turn = 0;
                
            }else{
                strcpy(server_message, "timeout");
                if (sendto(socket_desc, server_message, strlen(server_message), 0,
                    (struct sockaddr*)&client_addr, client_struct_length) < 0){
                    printf("Can't send\n");
                    exit(0);
                }
            }
        }
        else if(is_it_my_turn == 0){

            memset(client_message,0,2000);
            // alarm(10);
            if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
                (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                printf("Couldn't receive\n");
                exit(0);
            }

            if(strcmp(client_message,"timeout") == 0){
                printf("Your Turn!\n");
                is_it_my_turn = 1;
                continue;
            }
            printf("Client Choice: %s\n",client_message);

            //Update the board
            int choice = atoi(client_message);
            // printf("choice_int: %d",choice );
            choice -= 1;
            int row = ceil(choice/3);
            int col = choice%3;
            board[row][col] = 'O';

            printf("\n%s\n",drawBoard(board));
            // memset(server_message,0,2000);
            strcpy(server_message, drawBoard(board));
    
            if (sendto(socket_desc, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                printf("Can't send\n");
                exit(0);
            }

            /* Broadcast sendString in datagram to clients every 3 seconds*/
            if (sendto(sock, server_message, strlen(server_message), 0, (struct sockaddr *) 
                &broadcastAddr, sizeof(broadcastAddr)) != strlen(server_message))
                DieWithError("sendto() sent a different number of bytes than expected");

            is_it_my_turn = 1;
            printf("Your Turn\n");
            // alarm(0);
        }

        sleep(0.5);
    }

    // Close the socket:
    close(socket_desc);
}

void clientUDP(int game_port,int clientid_1,int clientid_2){
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];
    int server_struct_length = sizeof(server_addr);
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(game_port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    strcpy(client_message,"ready");
    // Send the message to server:
    if(sendto(socket_desc, client_message, strlen(client_message), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Unable to send message\n");
        return;
    }

    // Recieve the board 
    if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
        (struct sockaddr*)&server_addr, &server_struct_length) < 0){
        printf("Error while receiving server's msg\n");
        return;
    }

    //Print the board
    printf("\n%s\n", server_message);

    is_it_my_turn = 0;
    char buff[1024] = {0};
    signal(SIGALRM, timeout_handler);
    siginterrupt(SIGALRM, 1);

    while(1){
        if(is_it_my_turn == 0)
        {
            // alarm(10);
            // Recieve the board
            if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                (struct sockaddr*)&server_addr, &server_struct_length) < 0){
                printf("Error while receiving server's msg\n");
                // return;
            }

            if(strcmp(server_message,"timeout") == 0){
                printf("Your Turn!\n");
                is_it_my_turn = 1;
                continue;
            }

            //Print the board
            printf("\n%s\n", server_message);

            is_it_my_turn = 1;
            printf("Your Turn!\n");
        }else if(is_it_my_turn == 1){
            alarm(10);
            memset(buff,0,1024);
            int r = read(0, buff, 1024);
            // printf("Read: %d\n",r);
            alarm(0);

            // printf("buff: %s\n",buff);

            buff[strcspn(buff, "\n")] = 0;

            

            if(r > 0){
                memset(client_message,0,2000);
                strcpy(client_message,buff);
                // Send the message to server:
                if(sendto(socket_desc, client_message, strlen(client_message), 0,
                    (struct sockaddr*)&server_addr, server_struct_length) < 0){
                    printf("Unable to send message\n");
                    // exit(0);
                }

                memset(server_message,0,2000);
                // Receive the server's response:
                if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                    (struct sockaddr*)&server_addr, &server_struct_length) < 0){
                    printf("Error while receiving server's msg\n");
                    // exit(0);
                }

                if(strcmp(server_message,"Finished!") == 0){
                    printf("The game is finished!\n Check game_result.txt.");
                    break;
                }

                //Print the board
                printf("\n%s\n", server_message);

                is_it_my_turn = 0;
            }
            else{
                memset(client_message,0,2000);
                strcpy(client_message,"timeout");
                // Send the message to server:
                if(sendto(socket_desc, client_message, strlen(client_message), 0,
                    (struct sockaddr*)&server_addr, server_struct_length) < 0){
                    printf("Unable to send message\n");
                    // exit(0);
                }
            }
        }

    }
    
    // Close the socket:
    close(socket_desc);
}

void handle_broadcast(int game_port,int user_type,int clientid_1,int clientid_2,int fd){
    if (user_type == 1){
        int sock;                         /* Socket */
        struct sockaddr_in broadcastAddr; /* Broadcast address */
        char *broadcastIP;                /* IP broadcast address */
        unsigned short broadcastPort;     /* Server port */
        char *sendString;                 /* String to broadcast */
        int broadcastPermission;          /* Socket opt to set permission to broadcast */
        unsigned int sendStringLen;       /* Length of string to broadcast */

        broadcastIP = "0.0.0.0";            /* First arg:  broadcast IP address */ 
        broadcastPort = game_port-1000;    /* Second arg:  broadcast port */

        /* Create socket for sending/receiving datagrams */
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            DieWithError("socket() failed");

        /* Set socket to allow broadcast */
        broadcastPermission = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
            sizeof(broadcastPermission)) < 0)
            DieWithError("setsockopt() failed");

        /* Construct local address structure */
        memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
        broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
        broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);/* Broadcast IP address */
        broadcastAddr.sin_port = htons(broadcastPort);         /* Broadcast port */

        serverUDP(game_port,clientid_1,clientid_2,fd,sock,broadcastAddr);

    }
    else
        clientUDP(game_port,clientid_1,clientid_2);
}

void connect_game(int game_port){
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast Address */
    unsigned short broadcastPort;     /* Port */
    char recvString[1000]; /* Buffer for received string */
    int recvStringLen;                /* Length of received string */

    broadcastPort = game_port-1000;   /* First arg: broadcast port */

    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
        DieWithError("bind() failed");

    
    while(1){
        /* Receive a single datagram from the server */
        if ((recvStringLen = recvfrom(sock, recvString, 1000, 0, NULL, 0)) < 0)
            DieWithError("recvfrom() failed");

        recvString[recvStringLen] = '\0';
        printf("%s\n", recvString);    /* Print the received string */
    }
    
    
    close(sock);
}

void run(int port)
{
    int fd;
    char buff[1024] = {0};

    fd = connectServer(port);

    //Wait for ready
    while (1){
        read(0, buff, 1024);
        buff[strcspn(buff, "\n")] = 0;
        if(strcmp(buff,"ready") == 0){
            break;
        }
        else if(strcmp(buff,"get_games") == 0){
            send(fd, buff, strlen(buff), 0);
            memset(buff, 0, 1024);

            //Recieve game list from server
            recv(fd, buff, 1024, 0);

            printf("%s\n", buff);
        }else if(strcmp(buff,"connect") == 0){
            break;
        }
    }
    if(strcmp(buff,"ready") == 0){
        send(fd, buff, strlen(buff), 0);
        memset(buff, 0, 1024);

        //Recieve game port from server
        recv(fd, buff, 1024, 0);
        char * pch;
        pch = strtok (buff," ");
        int game_port, current_clientid,opponent_clientid;
        int user_type;

        int i = 0;
        while (pch != NULL)
        {
            switch(i)
            {
                case 0:
                    game_port = atoi(pch);
                    break;
                
                case 1:
                    current_clientid = atoi(pch);
                    break;
                
                case 2:
                    opponent_clientid = atoi(pch);
                    break;
                
                case 3:
                    user_type = atoi(pch);
                    break;
                
            }
            i += 1;
            pch = strtok (NULL, " ");
        }
        memset(buff, 0, 1024);
        printf("Port %d , Opponent Client ID: %d, Current Client ID: %d\n",game_port,opponent_clientid,current_clientid);

        //Start Broadcasting on game_port
        handle_broadcast(game_port,user_type,current_clientid,opponent_clientid,fd);
    }
    //Connect to a broadcast server
    else if(strcmp(buff,"connect") == 0){
        read(0, buff, 1024);
        buff[strcspn(buff, "\n")] = 0;

        int game_port = atoi(buff);

        connect_game(game_port);
    }


    close(fd);
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