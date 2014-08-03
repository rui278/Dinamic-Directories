#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "list.h"
#include "globals.h"

/** The list is ordered by the chronological order of arrival in the chat.
**/

/************************************************************************************************
    initList()
Arguments:
		inputData - string with the information of the user to be inserted in the list.
		name.surname;IP;tcpPort;udpPort;
Description: Given a string which format is the above specified, a new user is created
and the list is initialised.
*************************************************************************************************/
void initList (char * inputData){

	firstUser = createUser(inputData);

	/* set list's pointers to the first user. The first user in the list
	is the authorized DNS for user's surname. */
	myDns = firstUser;
	lastUser = firstUser;
	firstUser->nextUser = NULL;
	size = 1;
}

/************************************************************************************************
    initListFromUser()
Arguments:
		firstUser - struct of type user with the information of the user
Description: Given a user to be inserted, the list is initialised directly with it
*************************************************************************************************/
void initListFromUser (user * firstUser){

	/* set list's pointers to the first user. The first user in the list
	is the authorized DNS for user's surname. */

	myDns = firstUser;
	lastUser = firstUser;
	firstUser->nextUser = NULL;
	size = 1;
}

/************************************************************************************************
    insertInList()
Arguments: 
		inputData - string with the information of the user to be inserted in the list.
		name.surname;IP;tcpPort;udpPort;
Description: A new user is created given the string inputData. New users are inserted 
on top of last user. 
*************************************************************************************************/
void insertInList (char* inputData){

    user* newUser;

	newUser = createUser(inputData);

	newUser->nextUser = lastUser;
	lastUser = newUser;
	size++;
}

/************************************************************************************************
    removeFromList()
Arguments:
		name - string with the name of the user to be removed
Description: Given the name of the user to be removed, the list is travelled searching for 
the user which name is the one desired.
*************************************************************************************************/
int removeFromList (char* name){

	user *aux1, *aux2;

	aux2 = lastUser;
	if(lastUser != NULL)
        aux1 = lastUser->nextUser;

	/* Case: the user to be removed is the last user inserted */
	if (!strcmp(aux2->name, name)){

		lastUser = aux1;
		free(aux2);

        size --;
		return 1;
	}

	while (aux1 != NULL){

		if (!strcmp(aux1->name, name)){

			aux2->nextUser = aux1->nextUser;
			free(aux1);
			size--;

			return 1;
		}

		aux2 = aux1;
		aux1 = aux1->nextUser;
	}

	size--;
	return -1;

}

/************************************************************************************************
    findInList()
Arguments:
		nameSurname - a string not parsed, like, 'name.surname'
Description: In this function a user is looked up in the list. Starting in the last user
the list is travelled until the user is found. Case the user is not in list, the function 
returns NULL.
*************************************************************************************************/
user * findInList(char * nameSurname){

    char buffer[128];

    user * toFind;

    toFind = lastUser;


    memset(buffer,'\0', 128);
    while(toFind != NULL){

    	/*create a string of type 'name.surname' of each element of the list so it
    	can be compared with the input string*/
        strcpy(buffer, toFind->name);
        strcat(buffer, ".");
        strcat(buffer, toFind->surname);

        if(!strcmp(buffer, nameSurname)){
            return toFind;
        }
		toFind = toFind->nextUser;
    }

    return NULL;
}

/************************************************************************************************
    eraseList()
Description: When a user leaves the application, the list should be erased and the memory
freed.That is the reason of existance of this function.
*************************************************************************************************/
void eraseList(){

	user *aux1, *aux2;

	aux1 = lastUser;
	aux2 = lastUser;


	while(aux2 != NULL){
		aux2 = aux1->nextUser;
		free(aux1);
		aux1 = aux2;
	}

	lastUser->nextUser = NULL;
	lastUser = NULL;
	size = 0;
	return;
}
