#ifndef UDPCOMM_H_INCLUDED
#define UDPCOMM_H_INCLUDED

int handleUDP(int *udpSocket);
int commREG(char* buffer, int* udpSocket, struct sockaddr_in addr);
int BcastMyself(int joinSocket, struct sockaddr_in inAddr);
int acceptNew(char* buffer, struct sockaddr_in addr);
char* stringUNR(user MyUser);
char* stringDNS(user auxUser);
int unrFamily(char* sUNR);
int unrSA(char* sUNR, char * saAddr, int saPort);
int newSNP(char* sDNS, user auxUser);
int commDNS(int* udpSocket, struct sockaddr_in addr);
int commUNR(char* buffer, int* udpSocket, struct sockaddr_in addr);
int commQRY(int * socket, char * querie, struct sockaddr_in addr);

#endif
