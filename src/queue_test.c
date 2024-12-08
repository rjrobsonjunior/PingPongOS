#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

queue_t* create_element(int content){
	queue_t* e = (queue_t*) malloc(sizeof(queue_t));
	if(e == NULL)
		return NULL;
	e->id = content;
	return e;
}

queue_t* dequeue(queue_t **queue) {
    if (queue == NULL || *queue == NULL) {
        return NULL;
    }
    queue_t *queue_head = *queue;
    if (queue_head->next == NULL)
	{
        *queue = NULL;
    } 
	else 
	{
        *queue = queue_head->next;
		queue_head->prev->next= queue_head->next;
        (*queue)->prev = NULL;
    }
    queue_head->next = NULL;
    queue_head->prev = NULL;
    return queue_head;
}

int contains(queue_t **queue, int id)
{
    if (queue == NULL || *queue == NULL) {
        return NULL;
    }
	queue_t * queue_head = *queue;
	queue_t * queue_it = *queue;
	do{
		if(queue_it->id == id)
			return 1;
		queue_it = queue_it->next;
	}
	while(queue_head != queue_it);
	return 0;
}


void imprimir(queue_t *queue){
	if(queue == NULL)
		return 0;

	queue_t *it = queue;
	do
	{
		printf("Elemento: %d \n", queue->id);
		queue = queue->next;
	}
	while(queue!=it);
}



int main(){
	queue_t* queue;
	
	queue_t* e1= create_element(0);
	queue_t* e2= create_element(1);
	queue_t* e3= create_element(2);

	queue_append (&queue, e1);
	queue_append (&queue, e2);
	queue_append (&queue, e3);

	if(contains(&queue, 1))
		printf("Contains: 1\n");
	if(contains(&queue, 2))
		printf("Contains: 2\n");
	if(contains(&queue, 5))
		printf("Contains: 5\n");
	printf("%d \n", dequeue(&queue)->id);

	imprimir(queue);
}



