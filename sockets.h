#ifndef SOCKETS_H_INCLUDED
#define SOCKETS_H_INCLUDED

#define MAX(i,j) ((i)>=(j)?(i):(j))

void setupServerSockets(int *udp, int *tcp, int udpPort, int tcpPort);
void closeServers();

#endif
