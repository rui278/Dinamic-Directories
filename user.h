#ifndef USER_H_INCLUDED
#define USER_H_INCLUDED


typedef struct users {

    char name[50];
    char surname[50];
    char IP[50];
    char contact[256];

    int udpPort;
    int tcpPort;

    struct users * nextUser;
} user;

user* createUser(char* inputData);

#endif
