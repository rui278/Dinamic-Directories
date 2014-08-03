#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

/*from list.h*/

user * myDns;
user * lastUser;
user * firstUser;

int size;

/*from sockets.h*/

int udpSock;
int tcpSock;

/*from user.h*/

int snp;
user globalMyUser;

/*from commands.h*/

int saSocket;
int talkSock;

/*from commands.c*/

int inSession;
int connectToUser;
char * connectUserName;
char * connectIP;
int connecttcp;

/*from main.c*/
char *saPort;
char *saIP;



#endif
