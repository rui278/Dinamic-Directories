#ifndef SERVERS_H_INCLUDED
#define SERVERS_H_INCLUDED

int handleTCP(int *tcpSocket);
int handleChat(int *talkSock);
void ocupado(int newTCP);

#endif
