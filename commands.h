#ifndef COMANDS_H_INCLUDED
#define COMANDS_H_INCLUDED

int handleComands(user myUser, char * saAddr, char * saPort);

int joinCommand(user myUser, char * saAddr, int saPort);
int leaveCommand(user myUser, char * saAddr, int saPort);
int findCommand(char * buffer, char * saIp, int saPort);
int connectCommand(char * buffer, char * saIp, int saPort);
int chatCommand(char * command);
int disconnectCommand();
int lst();
int exitCommand();
void terminateLeave();

#endif
