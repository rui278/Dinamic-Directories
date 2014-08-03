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
#include "sockets.h"
#include "commands.h"
#include "udpComm.h"
#include "globals.h"


/************************************************************************************************
    handleComands()
Arguments:
        myUser - struct with 'my' information
        saAddr - IP address of the SA
        saPortChar - port of SA
Description: The messages typed by the user in the terminal are handled in this function.
The commands permitted are join, leave, find, connect, message, disconnect, exit, lst.
*************************************************************************************************/
int handleComands(user myUser, char * saAddr, char * saPortChar){

    char command[256];
    char * parseCommand;
    int retval, saPort, i=0;

    saPort = atoi(saPortChar);

    /*retrieves comand and stores to string command*/
    memset(command, '\0', 256);

    fgets(command, 256, stdin);

    for(i = 0; i < 256; i++){
        if(command[i] == '\n'){
            parseCommand = (char *) malloc((i+2)*sizeof(char));
            break;
        }
        if(command[i] == ' '){
            parseCommand = (char *) malloc((i+2)*sizeof(char));
            break;
        }
    }

   if(parseCommand == NULL){
        printf("\n>Malloc on ParseCommand not succecefull\n");
        fflush(stdout);
        free(parseCommand);
        exit(EXIT_FAILURE);
    }

    memset(parseCommand, '\0', (i+2));

    retval = sscanf(command, "%s", parseCommand);

    if(retval != 1){
        printf("\n>Error parsing command, please retry\n");
        free(parseCommand);
        return 1;
    }

    /*Switch Commands*/


    /*Join*/
    if(strcmp("join", parseCommand) == 0){
        if(joinCommand(myUser,saAddr, saPort) < 0){
            fprintf(stderr, "Join Error");
            free(parseCommand);
            exit(EXIT_FAILURE);
        }
        printf("\n>You have started a session\n");
        inSession = 1;
    }

    /*Leave*/
    else if(strcmp(parseCommand, "leave") == 0){
        if(leaveCommand(myUser, saAddr, saPort) < 0){
            fprintf(stderr, "\n>Leave Error>");
            free(parseCommand);
            return 0;
        }
        terminateLeave();
        inSession = 0;
    }

    else if(strcmp(parseCommand, "find") == 0){
        if(findCommand(command, saAddr, saPort) < 0){
            fprintf(stderr, "\n>Leave Error\n");
            free(parseCommand);
            return 0;
        }
    }

    else if(strcmp(parseCommand, "connect") == 0){
        if(connectCommand(command, saAddr, saPort) <= 0){
            fprintf(stderr, "\n>connect Error\n");
            free(parseCommand);
            return 0;
        }
    }
    else if(strcmp(parseCommand, "message") == 0){
        if(chatCommand(command) <= 0){
            fprintf(stderr, "\n>message Error\n");
            free(parseCommand);
            return 0;
        }
    }
    else if(strcmp(parseCommand, "disconnect") == 0){
        if(disconnectCommand() <= 0){
            fprintf(stderr, "\n>disconnect Error\n");
            free(parseCommand);
            return 0;
        }
    }
    /*Exit*/
    else if(strcmp(parseCommand, "exit") == 0){
        if(exitCommand() <= 0){
            fprintf(stderr, "\n>Exit Error>");
            free(parseCommand);
            return 0;
        }
    }

    else if(strcmp(parseCommand, "lst") == 0){
        if(lst() < 0){
            fprintf(stderr, "\n>lst Error>");
            free(parseCommand);
            return 0;
        }

    }

    free(parseCommand);

    return 1;
}



/************************************************************************************************
    joinCommand()
Arguments:
        myUser - struct with 'my' information
        saAddr - IP address of SA
        saPort - port of SA
Description: In this function, the join command is handled. Fisrt a new user must send the REG
message to SA. SA answers with a DNS message. Case the SNP is myself, the join is done. In case
the SNP is other user, then the user must join the rest of the family, starting to sent the REG
message to the SNP, receiving the LST message from the SNP and then, send the REG message to
every member of the family and wait for their OK.
*************************************************************************************************/
int joinCommand(user myUser, char * saAddr, int saPort){

    /*struct hostent *h;*/
    struct in_addr a;
    struct sockaddr_in addr;

    user * dnsUser;

    int joinSocket;
    int retval, outSize;

    int bcast = 1, i,n;

    socklen_t addrlen;

    char * output;
    char outputLarge[1024];
    char toParse[1024];
    char inbuffer[256];
    char userBuff[50];
    char answer[4];

    struct timeval timeout;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    /*Creates udp Socket to communicate with SA*/
    saSocket = socket(AF_INET, SOCK_DGRAM,0);

    if (setsockopt (saSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        perror("\n>setsockopt failed\n");

    dnsUser = (user*) malloc(sizeof(user));
    if(dnsUser == NULL){
        fprintf(stderr, "Malloc Error on dnsUser");
        return -1;
    }

    memset((void*)inbuffer, 0, 256);
    memset((void*)outputLarge, '\0', 1024);

    inet_pton(AF_INET, saAddr, (void*) &a);

    memset((void*)&addr, (int)'\0', sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(saPort);
    addr.sin_addr = a;

    outSize = strlen(myUser.contact)*sizeof(char) + sizeof("REG ") -1 + 2; /* -1 porque  ele conta com o \0 no size do REG*/

    output = (char*) malloc(outSize);

    /* Create output string */
    snprintf(output, outSize, "%s %s", "REG", myUser.contact);


    /*Sends Register message to SA server*/
    retval = sendto(saSocket, output, strlen(output)*sizeof(char), 0, (struct sockaddr*)&addr, sizeof(addr));

    if(retval == -1){
        perror("\n>Join Error\n");
        free(output);
        return -1;
    }

    addrlen = sizeof(addr);

    /*Retrieves response message from SA server*/

    retval = recvfrom(saSocket, inbuffer, 256, 0, (struct sockaddr*)&addr, &addrlen);

    if(retval == -1){
        perror("\n>Join recv Error\n");
        free(output);
        return -1;
    }

	inbuffer[retval] = '\0';

    retval = sscanf(inbuffer, "%s %[^.].%[^;];%[^;];%d", answer, dnsUser->name, dnsUser->surname, dnsUser->IP, &dnsUser->udpPort);
    if ((retval != 5) || (strcmp(answer, "DNS") != 0)){
        fprintf(stderr, "Error when matching SA's response to register.\n");
        free(output);
        return -1;
    }

    myDns = dnsUser;

    /*If user is authorised dns for username, than completes the user
    structure with details and then inserts it into surname users list.*/
    if (strcmp(dnsUser->name, myUser.name) == 0){

        dnsUser->tcpPort = myUser.tcpPort;
        memset(dnsUser->contact, '\0', 256);
        strcpy(dnsUser->contact, myUser.contact);
        initListFromUser(dnsUser);
        snp = 1;
        free(output);
        return 1;
    }

    /*If user is not authorized dns then will register with authorized
     dns and retrive surname users list*/

    /*create socket to talk to DNS*/

    joinSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (setsockopt (joinSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        perror("\n>setsockopt failed\n");

    /*Create Addr struct for DNS*/
    inet_pton(AF_INET, dnsUser->IP, (void*) &a);
    /*casts first usable name into in_addr struct form*/


    memset((void*)&addr, (int)'\0', sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(dnsUser->udpPort);
    addr.sin_addr = a;
    addrlen = sizeof(addr);

    retval = sendto(joinSocket, output, strlen(output)*sizeof(char), 0, (struct sockaddr*)&addr, sizeof(addr));

    if(retval == -1){
        perror("Join recv Error\n");
        close(joinSocket);
        free(output);
        return -1;
    }

    retval = recvfrom(joinSocket, outputLarge, 1024, 0, (struct sockaddr*)&addr, &addrlen);

    if(retval == -1){
        perror("Join recv Error\n");
        close(joinSocket);
        free(output);
        return -1;
    }

    retval = sscanf(outputLarge, "%[^\n]\n%[^ ]", answer, toParse);

    if(retval != 2){
        fprintf(stderr, "Could not Parse LST from SNP\n");
        close(joinSocket);
        free(output);
        return -1;
    }
    answer[3] = '\0';

    if(strcmp(answer, "LST") == 0){
        bcast = 1;

        while(bcast){
            n=-1;
            for(i = 0; i < 1023; i++){
                n++;
                if(toParse[i] == '\n'){

                    insertInList(userBuff);
                    memset(userBuff,'\0', 50);
                    n=-1;

                    if(toParse[i+1] == '\n'){
                        bcast = 0;
                        toParse[i+1] = '\0';
                        break;
                    }else{
                        continue;
                    }
                }

                userBuff[n%50] = toParse[i];
            }

            if(bcast){
                    retval = recvfrom(joinSocket, outputLarge, 1024, 0, (struct sockaddr*)&addr, &addrlen);

                    if(retval == -1){
                        perror("\n>Join->LST recv Error\n");
                        close(joinSocket);
                        free(output);
                        return -1;
                    }

                    retval = sscanf(outputLarge, "%[^ ]\n%[^ ]", answer, toParse);
                    answer[3] = '\0';
                    if (strcmp(answer, "LST") != 0){
                        fprintf(stderr, "\n>Bad format in Join->LST. Expected LST\n[RemainderUsers], got \"%s\" instead\n", answer);
                        close(joinSocket);
                        free(output);
                        return -1;
                    }
            }
        }

        retval = BcastMyself(joinSocket, addr);

        if(retval < 0){
            fprintf(stderr, "Error uppon Join->BcastMyself");
            free(output);
            close(joinSocket);
            return -1;
        }
    }

    free(output);
    close(joinSocket);
    return 1;
}


/************************************************************************************************
    leaveCommand()
Arguments:
        myUser - struct with 'my' information
        saAddr - IP address of SA
        saPort - port of SA
Description: The leave command has different behaviour, depending whether the user is the SNP
or not. Case it is the SNP, the user must unregister in SA and declare a new SNP. After that,
the new SNP is informed of his new role and each member of the family receive a UNR message.
If the user is not the SNP, he just needs to send UNR messages for every member of the family.
*************************************************************************************************/
int leaveCommand(user myUser, char * saAddr, int saPort){

    char sUNR[128], sDNS[128];
    char *dns, *unr;
    user* auxUser;

    if(saSocket <= 0){
        printf("\n>Whoops! Looks like you forgot to call join first..\n");
        return 1;
    }

    unr = stringUNR(myUser);
    strcpy(sUNR, unr);

    /*user that is not the SNP*/
    if(snp == 0){
        unrFamily((char*)sUNR);
    }

    else{
        if(size == 1){

            unrSA(sUNR, saAddr, saPort);
            return 1;
        }

        unrFamily((char*)sUNR);

        if (strcmp(globalMyUser.name, lastUser->name) == 0){
            auxUser = lastUser->nextUser;
        }
        else{
            auxUser = lastUser;
        }

        dns = stringDNS(*auxUser);
        strcpy(sDNS, dns);

        unrSA(sDNS, saAddr, saPort);

        newSNP(sDNS, *auxUser);

        return 1;

    }

    return 1;
}


/************************************************************************************************
    lst()
Description: Function that display the family members
*************************************************************************************************/
int lst(){

    user * userIter;

    if(saSocket <= 0){
        printf("\n>Whoops! Looks like you forgot to call join first..\n");
        return 1;
    }

    userIter = lastUser;

    while(userIter != NULL){
        userIter = userIter->nextUser;
    }

    return 1;
}

/************************************************************************************************
    findCommand()
Arguments:
        myUser - struct with 'my' information
        saIP - IP address of SA
        saPort - port of SA
Description: Finds requested user IP and TCP port.
*************************************************************************************************/
int findCommand(char * buffer, char * saIp, int saPort){

    char userToFind[128];
    char outBuffer[256];
    char inBuffer[256];
    char UserName[128];
    char IP[50];
    char answer [10];
    char givenName[50];
    char family[50];

    int saSock;
    int retval;
    int udpPort;
    int tcpPort;
    socklen_t len;

    struct sockaddr_in addr;
    struct hostent *h;
    struct in_addr *a;

    user * usr;

    if(saSocket <= 0){
        printf("\n>Whoops! Looks like you forgot to call join first..\n");
        return 1;
    }

    /*gets host's ip adress*/

    if((h=gethostbyname(saIp)) == NULL){
        perror("\n>Gethostbyname Error\n");
        return -1;
    }

    /*casts first usable name into in_addr struct form*/
    a = (struct in_addr*)h -> h_addr_list[0];

    memset((void*)&addr, '\0', sizeof(addr));
    addr.sin_port = htons(saPort);
    addr.sin_family = AF_INET;
    addr.sin_addr = *a;

    memset(userToFind, '\0', 128);

    if(connectToUser == 1){
        sscanf(buffer, "connect %[^\n]", userToFind);
    }else{
        sscanf(buffer, "find %[^\n]", userToFind);
    }

    saSock = socket(AF_INET, SOCK_DGRAM, 0);

    memset(outBuffer, '\0', 256);
    snprintf(outBuffer, 256, "QRY %s", userToFind);

    retval = sendto(saSock, outBuffer, strlen(outBuffer), 0, (struct sockaddr*)&addr, sizeof(addr));

    if(retval < 0 ){
        perror("\n>Error in QRY sending to SA\n");
        return -1;
    }

    len = sizeof(addr);

    memset(inBuffer, '\0', 256);
    retval = recvfrom(saSock, inBuffer, 256, 0, (struct sockaddr*)&addr, &len);

    if(retval < 0 ){
        perror("\n>ERRO NO QRY RECVING TO SA\n");
        return -1;
    }

    memset(UserName, '\0', 128);
    memset(answer, '\0', 10);
    memset(IP, '\0', 50);

    if((sscanf(inBuffer, "%[^ ] %[^;];%[^;];%d", answer, UserName, IP, &udpPort)) != 4){
        if(strcmp(answer, "FW") == 0){
            printf("\n>There seems to be no user by the name of %s\n", userToFind);
            return 1;
        }
        return -1;
    }

    if(strcmp(answer, "FW") != 0){
            printf("\n>Bad format. Please re-introduce the command\n");
            return -99;
    }

    /**REPEAT QUERY, BUT TO SNP. MUST WRITE SNP'S ANSWER!*/

    inet_pton(AF_INET,IP, (void*)a);

    addr.sin_port = htons(udpPort);
    addr.sin_family = AF_INET;
    addr.sin_addr = *a;

    memset(outBuffer, '\0', 256);
    memset(givenName, '\0', 50);
    memset(family, '\0', 50);
    snprintf(outBuffer, 256, "QRY %s", userToFind);


    sscanf(userToFind, "%[^.].%s", givenName, family);

    if(!((strcmp(family, globalMyUser.surname) == 0) && (snp == 1))){
        retval = sendto(saSock, outBuffer, strlen(outBuffer), 0, (struct sockaddr*)&addr, sizeof(addr));

        if(retval < 0 ){
            perror("\n>ERROR IN QRY SENDING TO SNP\n");
            return -1;
        }

        len = sizeof(addr);

        memset(inBuffer, '\0', 256);
        retval = recvfrom(saSock, inBuffer, 256, 0, (struct sockaddr*)&addr, &len);

        if(retval < 0 ){
            perror("\n>ERROR IN QRY RECVING TO SA\n");
            return -1;
        }

        memset(UserName, '\0', 50);
        memset(answer, '\0', 10);
        memset(IP, '\0', 50);

        if((sscanf(inBuffer, "%[^ ] %[^;];%[^;];%d", answer, UserName, IP, &tcpPort)) != 4){
            if(strcmp(answer, "RPL") == 0){
                printf("\n>There seems to be no user by the name of %s\n", userToFind);
            }
            return -99;
        }

        if(strcmp(answer, "RPL") != 0){
                printf("\n>Bad format. Please re-introduce the command\n");
                return -99;
        }
    }else{
        usr = findInList(userToFind);
        memset(IP, '\0', 50);
        strcpy(IP, usr->IP);
        tcpPort = usr->tcpPort;


        if((!strcmp(givenName, usr->name)) && (!strcmp(family, usr->surname))){
            connectUserName = UserName;
            connectIP = IP;
            connecttcp = tcpPort;

            if(connectToUser == 0){
                printf("\n>User %s Found at %s using port %d\n", UserName, IP, tcpPort);
            }
            close(saSock);
            return 1;
        }
    }

    if(!strcmp(userToFind, UserName)){
        connectUserName = UserName;
        connectIP = IP;
        connecttcp = tcpPort;

        if(connectToUser == 0){
            printf("\n>User %s Found at %s using port %d\n", UserName, IP, tcpPort);
        }
        close(saSock);
        return 1;
    }

    close(saSock);

    return 0;
}

/************************************************************************************************
    connectCommand()
Arguments:
        myUser - struct with 'my' information
        saIP - IP address of SA
        saPort - port of SA
Description: Starts a connection with the TCP socket
*************************************************************************************************/
int connectCommand(char * buffer, char * saIp, int saPort){

    struct in_addr a;
    struct sockaddr_in addr;
    struct timeval timeout;

    int tcpSock;
    int retval;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    connectToUser = 1;

    if(findCommand(buffer,saIp,saPort) <= 0){
        printf("\n>error finding peer to connect\n");
    }

    connectToUser = 0;

    tcpSock = socket(AF_INET, SOCK_STREAM, 0);

    if (setsockopt (tcpSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

    memset((void*)&addr, '\0', sizeof(addr));

    inet_pton(AF_INET,connectIP, (void*) &a);

    addr.sin_port = htons(connecttcp);
    addr.sin_family = AF_INET;
    addr.sin_addr = a;

    retval = connect(tcpSock, (struct sockaddr*)&addr, sizeof(addr));

    if(retval < 0){
        perror("\n>Error uppon connecting!\n");
    }

    talkSock = tcpSock;

    return 1;
}

/************************************************************************************************
    chatCommand()
Arguments:
        command -
Description:
*************************************************************************************************/
int chatCommand(char * command){

    char msg[1024];
    char msgPlusHeader[1152];
    int retval;

    if(strlen(command) > 1024){
        printf("\n>Sorry, max message size is 1024 characters. Please type again!\n");
        return 1;
    }

    sscanf(command, "message %[^\n]", msg);

    memset(msgPlusHeader, '\0', 1152);
    snprintf(msgPlusHeader, 1152, "MSS %s.%s;%s", globalMyUser.name, globalMyUser.surname, msg);


    retval = write(talkSock, msgPlusHeader, strlen(msgPlusHeader));

    if(retval < 0){
        perror("\n>message error!\n");
        return 0;
    }

return 1;
}

/************************************************************************************************
    disconnectCommand()
Description: closes the TalkSock and set it to -1 so the next user that tries to reach
me does not get a wrong busy message
*************************************************************************************************/
int disconnectCommand(){

    close(talkSock);
    talkSock = -1;

    return 1;
}


/************************************************************************************************
    exitCommand()
Description: Exit command checks if all the connections were close, case they were not,
close all the connections and frees memory. Then, it exits the program.
*************************************************************************************************/
int exitCommand(){

    if(talkSock != -1){
        disconnectCommand();
    }

    if(inSession != 0){
        leaveCommand(globalMyUser, saIP, atoi(saPort));
        terminateLeave();
        inSession = 0;
    }

    closeServers();

    printf("\n>Safely exiting\n");
    exit(EXIT_SUCCESS);
}



/************************************************************************************************
    terminateLeave()
Description: Function that is used to erase the list and close the socket opened in join.
*************************************************************************************************/
void terminateLeave(){
    eraseList();
    close(saSocket);
    saSocket = -42;
    return;
}
