/* Nolan Mullins
 * 
 * Basic linked list with sorting
 * 
 * Call init() before using a List
 * listRemove will return a reference to the data stored at that node
 */

#ifndef __MULLINSN_LIST__
#define __MULLINSN_LIST__

#include <stdlib.h>

typedef struct node
{
	struct node* next;
	void* data;
} Node;

typedef struct head
{
	Node* list;
} List;

void throwError(char* msg);
List* init();
void listAdd (List* head, void* data);
void* listGet(List* head, int index);
void listSet(List* head, int index, void* data);
void listInsert(List* head, void* data, int index);
int listSize(List* head);
void* listRemove(List* head, int index);
List* listClear(List* head, void (*des)(void* a));
void delHead(List* head);

int indexOf(List* head, int (*cmp)(void* a, void* b), void* search);
void insertSorted(List* head, int (*cmp)(void* a, void* b), void* data);

void sort(List* head, int (*cmp)(void* a, void* b));
void sort2(List* head, int (*cmp)(void* a, void* b));

/*//del
//Node* get(List* head, int index);*/
Node* createNode(void* data);


#endif