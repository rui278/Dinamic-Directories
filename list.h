#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

void initList (char * inputData);
void initListFromUser (user * firstUser);
void insertInList (char* inputData);
int removeFromList (char* nameSurname);
user * findInList(char * nameSurname);
void eraseList();

#endif
