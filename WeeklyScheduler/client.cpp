//
//  client.cpp
//  WeeklyScheduler
//
//  Created by Tanner Juby on 3/15/17.
//  Copyright © 2017 Juby. All rights reserved.
//

#include "client.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 3490           // The port the client will be connecting to
#define MAXDATASIZE 512     // Max number of bytes we can get at once

int main(int argc, char *argv[]) {
    
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    char sendbuf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr;  // Connector's address information
    
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }
    
    if ((he = gethostbyname(argv[1])) == NULL) {    // Get the host information
        perror("gethostbyname");
        exit(1);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    their_addr.sin_family = AF_INET;    // Host byte order
    their_addr.sin_port = htons(PORT);  // Short network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);    // Zero the rest of the struct
    
    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }
    
    printf("connection has been established with server.\n\n");
    printf("How to communicate with the server:\n\n");
    printf("%-25s%-130s\n", "ACTION", "MESSAGE TO SEND");
    printf("%-25s%-130s\n", "------", "---------------");
    printf("%-25s%-130s\n", "Login", "login <email> <password>");
    printf("%-25s%-130s\n", "Logout", "logout <key>");
    printf("%-25s%-130s\n", "Add Account", "add_acount <email> <password> <first_name> <last_name> <phone>");
    printf("%-25s%-130s\n", "Delete Account", "delete_account <key> <password>");
    printf("%-25s%-130s\n", "Edit Account", "edit_account <key> <current password> <email> <new password> <first_name> <last_name> <phone>");
    printf("%-25s%-130s\n", "Add Event", "add_event <key> <name> <date (MM-DD-YYYY)> <start_time (HH:SS)> <end_time (HH:SS)>");
    printf("%-25s%-130s\n", "Delete Event", "delete_event <key> <password> <event_id>");
    printf("%-25s%-130s\n", "Edit Event", "edit_event <key> <password> <event_id> <name> <date (MM-DD-YYYY)> <start_time (HH:SS)> <end_time (HH:SS)>");
    printf("%-25s%-130s\n", "Event By Time", "event_by_time <key> <password> <date (MM-DD-YYYY)> <time (HH:SS)>");
    printf("%-25s%-130s\n", "Events By Time Range", "events_by_time_range <key> <password> <start date (MM-DD-YYYY)> <start time (HH:SS)> <end time (HH:SS)>");
    
    printf("\nInstructions: \n\t1) Copy the MESSAGE TO SEND string into message field\n\t2) Replace all fields surrounded by '< >' with the value you want for that field \n\t\t(EXAMPLE: \"login <email> <password> would be\" ---> \"login myEmail@sample.com myPassword1234\")\n\t3) Press 'enter' and wait for response. \n\n ***** \nNOTE: Any action you wish to do with your account (besides login and create) you need your account KEY. This will be provided when you login or create an account \n***** \n\nInput: ");
    
    
    
    
    for (;;) {
        
        gets(sendbuf);
        numbytes = sizeof(sendbuf);
        sendbuf[numbytes] = '\0';
        
        if (numbytes == 0 || strcmp(sendbuf, "exit") == 0) {
            printf("Bye\n");
            break;
        } else {
            
            // Send the Data
            if ((numbytes = send(sockfd, sendbuf, sizeof(sendbuf), 0)) == -1) {
                perror("send");
                close(sockfd);
                exit(1);
            }
            
            sendbuf[numbytes] = '\0';
            printf("Sent: %s\n", sendbuf);

            // Recieve the Data
            
            if ((numbytes = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
                perror("recv");
                exit(1);
            }
            
            buf[numbytes] = '\0';
            printf("Recieved: %s \n \n\nInput: ", buf);
            
        }
    }
    
    close(sockfd);
    return 0;
    
}
