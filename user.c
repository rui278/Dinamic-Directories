#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"


/************************************************************************************************
    createUser()
Arguments:
        inputData - string with the information of the user in the type:
                    name.surname;IP;tcpPort;udpPort;
Description: Given a string, this function parse the information in it and create a new user
*************************************************************************************************/
user* createUser(char* inputData){

	user* newUser;
    int retval;

	newUser = (user*) malloc(sizeof(user));

	memset(newUser->name, '\0', 50);
    memset(newUser->surname, '\0', 50);
    memset(newUser->IP, '\0', 50);


	retval = sscanf(inputData, "%[^.].%[^;];%[^;];%d;%d", newUser->name, newUser->surname, newUser->IP, &newUser->tcpPort, &newUser->udpPort);
    if (retval != 5){
        printf("\n>Error parsing new User\n");
        exit(EXIT_FAILURE);
    }

    memset(newUser->contact, '\0', 256);
    sprintf(newUser->contact, "%s", inputData);

	return newUser;
}
