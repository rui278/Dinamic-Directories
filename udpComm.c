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
#include "udpComm.h"
#include "commands.h"
#include "globals.h"

/************************************************************************************************
    handleUDP()
Arguments:
        udpSocket - global variable which is the socket descriptor of the UDP socket
Description: Depending on the message that is received in the UDP socket different actions
must be taken.
*************************************************************************************************/
int handleUDP(int *udpSocket){

	int nread;
	char buffer[128];
	char command[128];

	struct sockaddr_in addr;
	socklen_t addrlen;

	memset((void*)&addr, (int) '\0', sizeof(addr));
	memset(buffer, '\0', 128);

	addrlen=sizeof(addr);

	/**/
	nread=recvfrom(*udpSocket, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
	if(nread==-1){
		perror("socket error");
        fflush(stderr);
        return 0;
    }
	sscanf(buffer, "%[^ ]", command);


	if(strcmp(command, "REG")==0){
		commREG(buffer, udpSocket, addr);
    }

    if(strcmp(command, "UNR")==0){
        commUNR(buffer, udpSocket, addr);
    }

    if(strcmp(command, "DNS")==0){
        commDNS(udpSocket, addr);
    }

	if(strcmp(command, "QRY")==0){
        commQRY(udpSocket, buffer, addr);
	}

    return 0;
}

/************************************************************************************************
    commREG()
Arguments:
        buffer - buffer where the string received is stored
        udpSocket - global UDP socket descriptor
        sockaddr_in addr - struct set in handleUDP
Description: A REG message is receive. If the user is the SNP, a list of all the users of the
family must be sent in return, as an answer to the request of joining. In the other hand, if
the user is not the SNP, the answer is just a simple OK.
*************************************************************************************************/
int commREG(char* buffer, int* udpSocket, struct sockaddr_in addr){


    socklen_t addrlen;

	int ret;
	char output[1024];
	char aux[256];
    char parseUser[128];
    char contactPlaceHolder[50];

	user* udpUser;

    memset(parseUser, '\0', sizeof(parseUser));
    memset(contactPlaceHolder,'\0', 50);

    addrlen=sizeof(addr);

    sscanf(buffer, "REG %s", parseUser);

	/* this function creates a new user and inserts it in the family list */
	insertInList(parseUser);

    udpUser = lastUser;

	/* snp variable to determine whether we are the authorized snp or not*/
	if(snp==1){

        memset(aux, '\0', sizeof(aux));
        memset(output, '\0', sizeof(output));

        strcpy(output, "LST\n");

		while(udpUser != NULL){

            strcpy(contactPlaceHolder,udpUser->contact);
            strcpy(aux, strcat(contactPlaceHolder, "\n"));
            memset(contactPlaceHolder, '\0', 50);

            if (strlen(output)+strlen(aux) < 1024){
                strcat(output, aux);
            }
            else{
                ret=sendto(*udpSocket, output, strlen(output), 0, (struct sockaddr*)&addr, addrlen);
                if(ret==-1){
                    perror("\n>Error sending LST\n");
                    return 0;
                }
                memset((void*)&output, (int) '\0', sizeof(output));
            }

			udpUser = udpUser->nextUser;
		}

        strcat(output, "\n");
        ret=sendto(*udpSocket, output, strlen(output), 0, (struct sockaddr*)&addr, addrlen);
        if(ret == -1){
                perror("\n>Error sending list termination\n");
                return 0;
        }

	}else{
        acceptNew(buffer,addr);
	}

    return 0;

}

/************************************************************************************************
    BcastMyself()
Arguments:
        joinSocket - socket used in join fuction
        addr - struct set in handleUDP
Description: When a user is 'joining', after the registration with the SNP and filling the list
with the users of the same family, it must send the REG to the rest of the family. This way,
it will travel the list sending to each user the REG message and waiting for the answer 'OK'.
*************************************************************************************************/
int BcastMyself(int joinSocket, struct  sockaddr_in addr){

    struct in_addr a;
    user * userInList;

    char buffer[256];
    char answer[3];

    int retVal;
    socklen_t addrlen;

    userInList = lastUser;

    while(userInList != NULL){
        if((strcmp(userInList->name, globalMyUser.name) != 0) && (strcmp(userInList->name, myDns->name) != 0)){
            inet_pton(AF_INET, userInList->IP, (void*) &a);
            /*casts first usable name into in_addr struct form*/

            memset((void*)&addr, (int)'\0', sizeof(addr));
            addr.sin_family=AF_INET;
            addr.sin_port=htons(userInList->udpPort);
            addr.sin_addr = a;


            memset(buffer, (int) '\0', 256);
            snprintf(buffer, 256, "REG %s", globalMyUser.contact);


            retVal = sendto(joinSocket, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, sizeof(addr));

            if(retVal < 0){
                perror("\n>Error in retVal Bcast\n");
                return -1;
            }


            memset(buffer, (int) '\0', 256);
            addrlen = sizeof(addr);
            retVal = recvfrom(joinSocket, buffer, 256, 0, (struct sockaddr*)&addr, &addrlen);

            if(retVal < 0){
                perror("\n>Error in RecvFrom Bcast\n");
                return -1;
            }

            retVal = sscanf(buffer, "%[^\n]", answer);
            answer[2] = '\0';

            if((retVal < 0) || (strcmp(answer, "OK") != 0)){
                fprintf(stderr,"\n>Error in brodcasting myself.\n");
                return (-1);
            }
        }
        userInList = userInList->nextUser;
    }

    return 0;
}

/************************************************************************************************
    acceptNew()
Arguments:
        buffer - string with the information of the user to be parsed
        sockaddr_in addr - contains address and ports for comunication
Description: The accpetance of a new memeber of the family is done in here
*************************************************************************************************/
int acceptNew(char* buffer, struct sockaddr_in addr){

    struct in_addr a;
    struct timeval timeout;

    char parseUser[50];
    char IPaddr[50];
    char ok[3];
    char nameThrowAway[50];
    int retval, tcpPortThrowAway, udpPort, sock;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    memset(IPaddr, '\0', 50);
    memset(nameThrowAway, '\0', 50);
    memset(parseUser, '\0', 50);

    retval = sscanf(buffer, "REG %s", parseUser);

    if(retval < 0){
        fprintf(stderr, "\n>Error accepting new user\n");
        return 0;
    }

    retval = sscanf(parseUser, "%[^;];%[^;];%d;%d", nameThrowAway, IPaddr, &tcpPortThrowAway, &udpPort);

    inet_pton(AF_INET, IPaddr, (void*) &a);

    strcpy(ok, "OK");

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

    if (sock < 0){
        perror("\n>Error creating socket\n");
        return 0;
    }

    retval = sendto(sock, ok, strlen(ok), 0, (struct sockaddr*)&addr, sizeof(addr));

    if(retval < 0){
        perror("\n>Error accepting new user\n");
        return 0;
    }

    printf("\n>Accepted a new user user!\n");

    close(sock);
    return 0;
}


/************************************************************************************************
    stringUNR()
Arguments:
        user MyUser - struct user, with the information of the user (my user)
Returns: string output - string necessary to unresgister myself
Description: Since this string can be necessery in different parts of the project, this function
was created so the string could be created when needed.
*************************************************************************************************/
char* stringUNR(user MyUser){

    char* output;

    output = (char*)malloc(128);

    memset(output, '\0', 128*sizeof(char)*sizeof(char));

    snprintf(output, 128, "%s %s.%s", "UNR", MyUser.name, MyUser.surname);

    return output;
}

/************************************************************************************************
    stringDNS()
Arguments:
        user auxUser - struct user of the user to be nominated new SNP
Returns: string output - string necessary to nominate the new SNP
Description: Since this string can be necessery in different parts of the project, this function
was created so the string could be created when needed.
*************************************************************************************************/
char* stringDNS(user auxUser){

    char* output;

    output = (char*) malloc(128);

    memset(output, '\0', 128*sizeof(char));

    snprintf(output, 128, "DNS %s.%s;%s;%d", auxUser.name, auxUser.surname, auxUser.IP, auxUser.udpPort);

    return output;
}

/************************************************************************************************
    unrFamily()
Arguments:
        sUNR - string to unregister the user
Returns:
Description: In the leave command, the user leaving let the rest of the family know that he/she
is leaving. In this function the user sends the UNR message to every member of the family.
*************************************************************************************************/
int unrFamily(char* sUNR){

    user* userInList;
    char buffer[8];
    char answer[3];
    int sock, recVal, sendVal;

    socklen_t addrlen;

    struct timeval timeout;
    struct sockaddr_in addr;


    userInList = lastUser;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);


    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("setsockopt failed\n");



    while(userInList != NULL){

        if(strcmp(userInList->name, globalMyUser.name) == 0){
            userInList = userInList->nextUser;
        }

        else{

            memset((void*)&addr, (int)'\0', sizeof(addr));
            inet_pton(AF_INET, userInList->IP, (void*) &(addr.sin_addr));
            addr.sin_family=AF_INET;
            addr.sin_port=htons(userInList->udpPort);

            addrlen=sizeof(addr);

            sendVal=sendto(sock, sUNR, strlen(sUNR), 0, (struct sockaddr*)&addr, addrlen);
            if(sendVal==-1) {
                perror("\n>SendTo Error while unregistering\n");
                return -1;
            };

            recVal=recvfrom(sock, buffer, 8, 0, (struct sockaddr*)&addr, &addrlen);
            if(recVal < 0){
                perror("\n>recvFrom Error while unregistering\n");
                return -1;
            }

            recVal = sscanf(buffer, "%[^\n]", answer);
            answer[2] = '\0';

            if((recVal < 0) || (strcmp(answer, "OK") != 0)){
                fprintf(stderr,"\n>Error in unregister\n");
                return -1;
            }

            userInList = userInList->nextUser;
        }

    }

    close(sock);

    return 0;
}

/************************************************************************************************
    unrSA()
Arguments:
        sUNR - string to unresgister
        saAddr - the address of the SA (IP of the SA)
        saPort - port of SA
Description: This function is used to unresgiter a user directly to the SA. It is used when the
user is the only member of the family.
*************************************************************************************************/
int unrSA(char* sUNR, char * saAddr, int saPort){

    /*socklen_t addrlen;*/

    struct sockaddr_in addr;
    struct hostent *h;
    struct in_addr *a;

    int retval, recVal;
    char buffer[8];
    char answer[3];
    socklen_t addrlen;

    if((h=gethostbyname(saAddr)) == NULL){
        perror("Gethostbyname Error");
        return -1;
    }

    a = (struct in_addr*)h -> h_addr_list[0];

    memset((void*)&addr, (int)'\0', sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(saPort);
    addr.sin_addr = *a;

    retval = sendto(saSocket, sUNR, strlen(sUNR), 0, (struct sockaddr*)&addr, sizeof(addr));

    if (retval < 0){
        perror("\n>sendto() error\n");
    }
    if (strlen(sUNR) != (size_t)retval) {
        fprintf(stderr, "\n>sendto() unsuccessful. Return code: %i\n", retval);
    }

    addrlen=sizeof(addr);
    recVal=recvfrom(saSocket, buffer, 8, 0, (struct sockaddr*)&addr, &addrlen);


            if(recVal < 0){
                perror("\n>Error in RecvFrom\n");
                return -1;
            }

            recVal = sscanf(buffer, "%[^\n]", answer);
            answer[2] = '\0';

            if((recVal < 0) || (strcmp(answer, "OK") != 0)){
                fprintf(stderr,"\n>Error in unregister\n");
                return -1;
            }

    return 1;

}

/************************************************************************************************
    newSNP()
Arguments:
        sDNS - string to nominate the new SNP, created in function stringDNS
        auxUser - struct user with the information of the user to be nominated new SNP
Description: Before the actual SNP leaves, it nominate a new SNP, inform the new SNP of the
new role it is playing and waits for the 'OK' message from the new SNP.
*************************************************************************************************/
int newSNP(char* sDNS, user auxUser){

    struct sockaddr_in addr;
    struct timeval timeout;
    int sock, recVal, sendVal;
    char buffer[8];
    char answer[3];
    socklen_t addrlen;


    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);


    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

    addrlen=sizeof(addr);

    inet_pton(AF_INET, auxUser.IP, (void*)&(addr.sin_addr));

    memset((void*)&addr, (int)'\0', sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(auxUser.udpPort);

    sendVal = sendto(sock, sDNS, strlen(sDNS), 0, (struct sockaddr*)&addr, addrlen);

    if (sendVal < 0){
        perror("\n>sendto() error\n");
        return -1;
    }
    if (strlen(sDNS) != (size_t)sendVal) {
        fprintf(stderr, "\n>sendto() unsuccessful. Return code: %i\n", sendVal);
    }

    recVal=recvfrom(sock, buffer, 8, 0, (struct sockaddr*)&addr, &addrlen);
    if(recVal < 0){
        perror("\n>Error in RecvFrom unrFamily\n");
        return -1;
    }

    recVal = sscanf(buffer, "%[^\n]", answer);
    answer[2] = '\0';

    if((recVal < 0) || (strcmp(answer, "OK") != 0)){
        fprintf(stderr,"\n>Error in unregister\n");
        return -1;
    }

    return 1;
}

/************************************************************************************************
    commDNS()
Arguments:
        udpSocket - the socket to connect with the other users
        sockaddr_in addr - contains address and ports for comunication
Description: When the user is nominated new SNP, it should inform the previous SNP that
the message was handled sending back a 'OK' message.
*************************************************************************************************/
int commDNS(int* udpSocket, struct sockaddr_in addr){

    char ok[3];
    int ret;
    socklen_t addrlen;

    addrlen = sizeof(addr);

    strcpy(ok, "OK");
    snp = 1;

    ret=sendto(*udpSocket, ok, strlen(ok), 0, (struct sockaddr*)&addr, addrlen);
    if (ret < 0){
        perror("sendto() error\n");
        return -1;
    }
    if (strlen(ok) != (size_t)ret) {
        fprintf(stderr, "sendto() unsuccessful. Return code: %i\n", ret);
    }

    return 1;

}

/************************************************************************************************
    commUNR()
Arguments:
        buffer - string received from the user that wants to unregister
        udpSocket - socket to communicate with the user
        sockaddr_in addr - contains address and ports for comunication
Description: When a user recieves a message UNR an answer should be sent stating 'OK'
*************************************************************************************************/
int commUNR(char* buffer, int* udpSocket, struct sockaddr_in addr){

    char UNRname[16], UNRsurname[16];
    char ok[3];
    int ret;
    socklen_t addrlen;

    addrlen = sizeof(addr);

    /*parse do buffer*/
    sscanf(buffer, "UNR %[^.].%s", UNRname, UNRsurname);

    removeFromList(UNRname);

    strcpy(ok, "OK");

    ret=sendto(*udpSocket, ok, strlen(ok), 0, (struct sockaddr*)&addr, addrlen);

    if(ret < 0){
        perror("\n>Error leaving\n");
        return 1;
    }

    return 1;


}

/************************************************************************************************
    commQRY()
Arguments:
        socket -
        querie -
        sockaddr_in addr - contains address and ports for comunication
Description: The SNP asks SA or another SNP of the localisation of a spacific user
*************************************************************************************************/
int commQRY(int * socket, char * querie, struct sockaddr_in addr){



    char userName[128];
    char outBuffer[256];
    int retval;

    socklen_t len;

    user * userInList;


    memset(userName, '\0', 128);
    sscanf(querie, "QRY %s", userName);

    memset(outBuffer, '\0', 256);

    userInList = findInList(userName);

    if(userInList != NULL){
        snprintf(outBuffer, 256, "RPL %s.%s;%s;%d", userInList->name, userInList->surname, userInList->IP, userInList->tcpPort);
    }else{
        snprintf(outBuffer, 256, "RPL");
    }

    len = sizeof(addr);
    retval = sendto(*socket, outBuffer, strlen(outBuffer), 0, (struct sockaddr*) &addr, len);

    if(retval <0){
        perror("\n>No can send bro...\n");
        return-1;
    }

    return 1;
}
