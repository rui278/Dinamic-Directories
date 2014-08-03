#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "user.h"
#include "sockets.h"
#include "commands.h"
#include "comm.h"
#include "udpComm.h"
#include "globals.h"


user parseArgs(int argc, char ** argv, char * nameSurname);
void setDefaults(user * myUser);
void switchOptions(char flag, char * argv, user * MyUser);
void sighandler(int sig){

    if(sig == SIGINT){
        exitCommand();
        exit(EXIT_SUCCESS);
    }
    return;
}


int main (int argc, char **argv) {

    /* Declare variables */

    char *nameSurname;

    int udpSocket, tcpSocket;
    int maxFd, counter;
    int n = -1;

    fd_set socketSet;
    user myUser;
    inSession = 0;

    srand (time(NULL));

    nameSurname = (char*) malloc(128);
    myUser = parseArgs(argc, argv, nameSurname);
    /* Arguments and parsing*/


    sprintf(myUser.contact, "%s.%s;%s;%d;%d", myUser.name, myUser.surname, myUser.IP, myUser.tcpPort, myUser.udpPort);

    /*globals*/
    saSocket = 0;
    connectToUser = 0;

    /*bind signal Ctrl+C to signalhandler*/
     if (signal(SIGINT, sighandler) == SIG_ERR)
            printf("\ncan't catch SIGINT\n");

    /* Create sockets */

    setupServerSockets(&udpSocket, &tcpSocket, myUser.udpPort, myUser.tcpPort);

    maxFd = MAX(udpSocket, tcpSocket);
    snp = 0;
    globalMyUser = myUser;
    talkSock = -1;

    while(1) {
        maxFd = MAX(udpSocket, tcpSocket);
        FD_ZERO(&socketSet);

        FD_SET(udpSocket, &socketSet);
        FD_SET(tcpSocket, &socketSet);
        FD_SET(STDIN_FILENO, &socketSet);

        if(talkSock != -1){
            maxFd = MAX(maxFd, talkSock);
            FD_SET(talkSock, &socketSet);
        }

        printf("\n>");
        fflush(stdout);

        counter = select(maxFd+1, &socketSet, (fd_set*)NULL, (fd_set*)NULL, (struct timeval*)NULL);
        if (counter < 0) {
            printf("\n>Erro no select! \n");
            continue;
        }

        /* check which descriptors are "selected" */

        if (FD_ISSET(STDIN_FILENO,&socketSet)) {
            if(handleComands(myUser, saIP, saPort) < 0){
            	printf("\n>Could not handle input :( ! \n. Continuing. Please try again! \n");
            }

        }

        else if (FD_ISSET(udpSocket,&socketSet)) {
            handleUDP(&udpSocket);
        }

        else if (FD_ISSET(tcpSocket,&socketSet)) {
            n = handleTCP(&tcpSocket);
            if(n != -1){
                talkSock = n;
            };
        }

        else if (FD_ISSET(talkSock, &socketSet)) {
            handleChat(&talkSock);
        }


    }

    exit(EXIT_SUCCESS);
}

/************************************************************************************************
    parseArgs()
Arguments:
        argc - number of arguments
        argv - arguments. in the form of an array of strings.
        namesurname - string do tipo name.surname
Returns: user - struct of type user, containing the information of the user
Description: Since there are optional arguments of the function, this function sets the default arguments
             and changes them acording to user flags.
*************************************************************************************************/
user parseArgs(int argc, char ** argv, char * namesurname){

    user myUser;

    memset(myUser.name, '\0', 50);
    memset(myUser.surname, '\0', 50);
    memset(myUser.IP, '\0', 50);
    memset(myUser.contact, '\0', 256);

    if (argc < 3 || argc > 11){
        printf("\n>usage: dd name.surname IP [-t talkport] [-d dnsport] [-i saIP] [-p saport]\n");
        exit(EXIT_FAILURE);
    }

    setDefaults(&myUser);

    /**deppending on the number of arguments, the arguments corresponding to the selected flags will be replaced*/
    switch(argc){
        case 5:
            switchOptions(argv[3][1], argv[4], &myUser);
            break;
        case 7:
            switchOptions(argv[3][1], argv[4], &myUser);
            switchOptions(argv[5][1], argv[6], &myUser);
            break;
        case 9:
            switchOptions(argv[3][1], argv[4], &myUser);
            switchOptions(argv[5][1], argv[6], &myUser);
            switchOptions(argv[7][1], argv[8], &myUser);
            break;
        case 11:
            switchOptions(argv[3][1], argv[4] , &myUser);
            switchOptions(argv[5][1], argv[6] , &myUser);
            switchOptions(argv[7][1], argv[8] , &myUser);
            switchOptions(argv[9][1], argv[10], &myUser);
            break;
    }

    sprintf(myUser.IP, "%s", argv[2]);
    memset(namesurname, '\0',128);
    namesurname = argv[1];
    sscanf(namesurname, "%[^.].%s", myUser.name, myUser.surname);

    return myUser;
}

/************************************************************************************************
    setDefaults()
Arguments:
        user *myUser - a placeholder user to be returnded and turned into globalMyUser
Returns: void
Description: sets return arguments for  tcp and udp ports, SA port (58000) and SA IP (tejo.ist.utl.pt)
*************************************************************************************************/
void setDefaults(user * myUser){

    struct in_addr *a;
    struct hostent *h;

    saPort = (char*) malloc(15*sizeof(char));
    memset(saPort, '\0', 15);

    myUser->tcpPort = 1337 + (rand() % 150);
    myUser->udpPort = 9001 + (rand() % 150);
    strcpy(saPort, "58000");

    h = gethostbyname("tejo.ist.utl.pt");
    a = (struct in_addr *)h->h_addr_list[0];

    saIP = (char*) malloc(50*sizeof(char));
    memset(saIP, '\0', 50);

    inet_ntop(AF_INET, (void*) a, saIP, 50);
    return;
}

/************************************************************************************************
    switchOptions()
Arguments: flag, argv, struct user

Returns:    void
Description: deppending on the flag that was set, this replaces the corresponding default argument
             with a user chosen value.
*************************************************************************************************/
void switchOptions(char flag, char * argv, user * myUser){

    switch(flag){
        case 't':
            myUser->tcpPort = atoi(argv);
            break;
        case 'u':
            myUser->udpPort = atoi(argv);
            break;
        case 'i':
            saIP = argv;
            break;
        case 'p':
            saPort = argv;
            break;
    }
    return;
}
