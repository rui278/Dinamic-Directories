#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "user.h"
#include "list.h"
#include "comm.h"
#include "globals.h"


/************************************************************************************************
    handleTCP()
Arguments:
        tcpSocket - socket created for the tcp communications
Description: In this function the accept of the TCP socket is done case a chat is not alread
running with other user. In this case, a busy signal is sent back.
*************************************************************************************************/
int handleTCP(int *tcpSocket){

	struct sockaddr_in addr;
	int newTCP;
	socklen_t  addrlen;

	memset((void*)&addr, (int) '\0', sizeof(addr));

	addrlen=sizeof(addr);
	if((newTCP=accept(*tcpSocket,(struct sockaddr*)&addr, &addrlen)) <0){
		perror("\n>accept failed\n");
		return -1;
	}

	if(talkSock != -1){
        ocupado(newTCP);
        return -1;
	}

    printf("\n>Entered conversation\n");

	return newTCP;
}


/************************************************************************************************
    handleChat()
Arguments:
        talkSock - socket for the chat
Description: message received handled in this function.
*************************************************************************************************/
int handleChat(int *talkSock){

	int n = 0;
	char buffer[1152];
    char header[128];

    memset(buffer, '\0', 1152);
    memset(header, '\0', 128);

	n=read(*talkSock, buffer, 1152);

    if(n < 0){
        perror("\n>Read failed\n");
        printf("\n>Leaving conversation \n");
        close(*talkSock);
        *talkSock = -1;
        return 1;
    }
    if(n == 0){
        printf("\n>other user has left the conversation.\n");
        printf("\n>Leaving conversation \n");
        close(*talkSock);
        *talkSock = -1;
        return 1;
    }

    sscanf(buffer, "MSS %[^;];%[^\n]", header, buffer);

	printf("\n>message from %s: %s\n", header, buffer);

    return 1;
}


/************************************************************************************************
    ocupado()
Arguments:
        newTCP -
Description: Busy message sent when the user is already in a chat and another tries to reach him.
*************************************************************************************************/
void ocupado(int newTCP){

    int n = 0;
	char buffer[1152];

    memset(buffer, '\0', 1024);


    snprintf(buffer, 1152, "MSS %s.%s;", globalMyUser.name, globalMyUser.surname);

    strcat(buffer, "Sorry, this client is busy");

    n = write(newTCP, buffer, strlen(buffer));

    if(n < 0){
        perror("Read failed\n");
		return;
    }

    close(newTCP);
    newTCP = -1;
    return;
}

