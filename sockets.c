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
#include "commands.h"
#include "globals.h"


/************************************************************************************************
    setupServerSockets()
Arguments:
		udp - udp socket
		tcp - tcp socket
		udpPort - port to bind udp server
		tcpPort - port to bind tcp server
Description: Sets up the SNP and SC servers using a upd and tcp sockets respectively.
             Called in main, both sockets are added to select.
*************************************************************************************************/
void setupServerSockets(int *udp, int *tcp, int udpPort, int tcpPort){


	struct sockaddr_in udpAddr, tcpAddr;


	/* UDP SERVER */

	udpSock = socket(AF_INET, SOCK_DGRAM, 0);

	if(udpSock < 0){
		printf("\n>Erro a criar o udp\n");
		exit(EXIT_FAILURE);
	}

	memset((void*)&udpAddr, (int)'\0', sizeof(udpAddr));
	udpAddr.sin_family=AF_INET;
	udpAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	udpAddr.sin_port=htons(udpPort);

	if (bind(udpSock, (struct sockaddr*)&udpAddr, sizeof(udpAddr))==-1){
		printf("\n>Can not bind udp Addr\n>Please use a different UDP port");
		exit(EXIT_FAILURE);
	}


	/* TCP SERVER */
	tcpSock = socket ( AF_INET, SOCK_STREAM, 0);

	if(tcpSock < 0){
		printf("\n>Erro a criar o tcp\n");
		exit(EXIT_FAILURE);
	}

	memset((void*)&tcpAddr, (int)'\0', sizeof(tcpAddr));
	tcpAddr.sin_family=AF_INET;
	tcpAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	tcpAddr.sin_port=htons(tcpPort);


	if (bind(tcpSock, (struct sockaddr*)&tcpAddr, sizeof(tcpAddr))==-1){
		printf("\n>Can not bind TCP Addr\n>Please use a different TCP port");
		exit(EXIT_FAILURE);
	}

	if (listen(tcpSock, 5)==-1){
		perror("Could not listen to tcp");
		exit(EXIT_FAILURE);
	}

	*udp = udpSock;
	*tcp = tcpSock;


	return;
}


/************************************************************************************************
    closeServers()
Description: The only purpose of this function is to close the servers. Called in handleExit.
*************************************************************************************************/
void closeServers(){
	close(udpSock);
	close(tcpSock);
}
