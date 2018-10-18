#include <stdlib.h>
#include <stdio.h>
#include <list.h>
/*List -> Node */

/*restricted functions { */
void* destroyNode(struct node* node)
{
	void* tmp = node -> data;
	free(node);
	return tmp;
}
void throwError(char* msg)
{
	printf("%s\n", msg);
	exit(0);
}
Node* createNode(void* data)
{
	Node* newNode = malloc(sizeof(Node));
	newNode->data = data;
	newNode->next = NULL;
	return newNode;
}
void delHead(List* head)
{
	if (listSize(head) > 0)
		printf("List Size on head deletion: %d\n", listSize(head));
	free(head);
}
/*can be optimized once code is checked*/
Node* get(List* head, int index)
{
	if (index < 0)
		throwError("The fuck man, index < 0???");
	if (head == NULL)
		throwError("Null pointer exception");
	Node* list = head->list;
	int a;
	for (a = 0; a < index; a++)
	{
		if (list == NULL)
			throwError("index out of bounds");
		list = list->next;
	}
	if (list == NULL)
		throwError("index out of bounds");
	return list;
}
/*}*/

/*Used to init the list*/
List* init()
{
	List* newHead = malloc(sizeof(List));
	newHead->list = NULL;
	return newHead;
}

void listAdd (List* head, void* data)
{
	if (head == NULL)
		throwError("Null pointer exception");
	Node* newNode = createNode(data);
	if (head->list == NULL)
	{
		head->list = newNode;
		return;
	}
	Node* list = head->list;
	while (list->next != NULL)
		list = list->next;
	list->next = newNode;
}

/*Will return a reference to the data stored in the node*/
void* listRemove(List* head, int index)
{
	if (head == NULL || head->list == NULL)
		throwError("Null pointer exception");
	Node* list = head->list;
	if (index == 0)
	{
		Node* tmp = head->list;
		head->list = list->next;
		return destroyNode(tmp);
	}
	list = get(head, index-1);
	if (list->next == NULL)
		throwError("index out of bounds");
	Node* tmp = list->next;
	list->next = list->next->next;
	return destroyNode(tmp);
}

/*will return a reference to the data stored at the index node*/
void* listGet(List* head, int index)
{
	return get(head, index)->data;
}

void listSet(List* head, int index, void* data)
{
	get(head, index)->data = data;
}

void listInsert(List* head, void* data, int index)
{
	if (head == NULL)
		throwError("Null pointer exception");
	if (index == 0)
	{
		Node* tmp = head->list;
		head->list = createNode(data);
		head->list->next = tmp;
		return;
	}
	Node* list = get(head, index-1);
	Node* tmp = list->next;
	list->next = createNode(data);
	list->next->next = tmp;
}

int listSize(List* head)
{
	if (head == NULL)
		return 0;
	int s = 0;
	Node* list = head->list;
	while (list != NULL)
	{
		s++;
		list = list->next;
	}
	return s;
}

List* listClear(List* head, void (*des)(void* a))
{
	if (head==NULL)
		return NULL;
	Node* tmp = head->list;
	while (tmp != NULL)
	{
		Node* tmp2 = tmp;
		tmp = tmp->next;
		des(tmp2->data);
		free(tmp2);
	}
	free(head);
	return NULL;
}

/*good enough for a linked list*/
void sort(List* head, int (*cmp)())
{
	if (head==NULL)
		throwError("Null pointer exception");
	int size = listSize(head);
	int a;
	for(a = 0; a < size-1; a++)
	{
		Node* index = head->list;
		/*swap on head*/
		if (cmp(index->data, index->next->data) > 0)
		{
			Node* tmp = index->next->next;
			head->list = index->next;
			index->next->next = index;
			index->next = tmp;
		}
		index = head->list;
		int b;
		for (b = 1; b < size-1-a; b++)
		{
			Node* i = index->next;
			Node* j = index->next->next;
			if (cmp(i->data, j->data) > 0)
			{
				Node* tmp = j->next;
				index->next = j;
				j->next = i;
				i->next = tmp;
			}
			index = index->next;
		}
	}
}

int indexOf(List* head, int (*cmp)(void* a, void* b), void* search)
{
	if (head == NULL)
		return -1;
	Node* tmp = head->list;
	int a = 0;
	while (tmp != NULL)
	{
		if (cmp(tmp->data, search) == 0)
			return a;
		a++;
		tmp = tmp->next;
	}
	return -1;
}

void insertSorted(List* head, int (*cmp)(void* a, void* b), void* data)
{
	if (head == NULL)
		return;
	Node* tmp = head->list;
	int index = 0;
	while (tmp != NULL)
	{
		if (cmp(tmp->data, data) >= 0)
		{
			listInsert(head, data, index);
			return;
		}
		tmp = tmp->next;
		index++;
	}
	listAdd(head, data);
}
