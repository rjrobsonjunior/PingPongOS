#include "ppos.h"
#include "ppos-core-globals.h"
#include "ppos-disk-manager.h"
//#define DEBUG

// ****************************************************************************
// Coloque as suas modificações aqui, 
// p.ex. includes, defines variáveis, // estruturas e funções
#include <stdlib.h>
// Implementações MUTEX
#define TRUE 1
#define FALSE 0

void task_wait(semaphore_t *s);
int queue_contains(queue_t **queue, int id);
queue_t* create_element(task_t *task);
queue_t* dequeue(queue_t **queue);


#ifdef PREEMPT_MODE
    
    int sem_create(semaphore_t* s, int value) 
    {
        if (s == NULL) return -1;
        PPOS_PREEMPT_DISABLE;
        s->queue = NULL;
        s->value = value;
        s->active = 1;
        PPOS_PREEMPT_ENABLE;
        return 0;
    }

    
    int sem_down(semaphore_t* s) 
    {
        if (s == NULL) return -1;
        if (s->active == 0) return -1;

        PPOS_PREEMPT_DISABLE;
        s->value--;
        if (s->value < 0) 
        {
            task_suspend(taskExec, &(s->queue));
            PPOS_PREEMPT_ENABLE;
            task_yield();
        }
        else
        {
            PPOS_PREEMPT_ENABLE;
        }
        return 0;
    }


    int sem_up(semaphore_t* s) 
    {
        if (s == NULL) return -1;
        if (s->active == 0) return -1;
        PPOS_PREEMPT_DISABLE; 

        s->value++;
        if (s->value <= 0) {
            task_resume(s->queue);
        }

        PPOS_PREEMPT_ENABLE;
        return 0;
    }

    int sem_destroy(semaphore_t* s) 
    {
        if (s == NULL) return -1;
        if (s->active == 0) return -1;
        
        PPOS_PREEMPT_DISABLE;
        s->active = 0;
        while (s->queue != NULL) {
            task_resume(s->queue);
        }

        PPOS_PREEMPT_ENABLE;
        return 0;
    }
#else
    int sem_create(semaphore_t *s, int value)
    {
        if (s == NULL) return -1;
        s->queue = NULL;
        s->value = value;
        s->active = 1;
        
        if (mutex_create(&(s->mutex_union.mutex)) != 0)
            return -1;
        return 0;  
    }


    int sem_down(semaphore_t *s)
    {
        if (s == NULL) return -1;
        if (s->active == 0) return -1;
        
        if(mutex_lock(&(s->mutex_union.mutex)) == -1) return -1;

        before_sem_down(s);
        s->value--;
        if (s->value < 0) {
            task_suspend(taskExec, (task_t**)&(s->queue));
            if(mutex_unlock(&(s->mutex_union.mutex)) == -1) return -1;
            task_yield();
            return 0;
        }
        if(mutex_unlock(&(s->mutex_union.mutex)) == -1) return -1;

        after_sem_down(s);
        return 0;
    }

    int sem_up(semaphore_t *s)
    {
        if (s == NULL) return -1;
        if (mutex_lock(&(s->mutex_union.mutex))) return -1;

        s->value++;
        if (s->value <= 0) {
            task_resume(s->queue);
        }
        mutex_unlock(&(s->mutex_union.mutex));
        return 0;
    }

    int sem_destroy(semaphore_t *s)
    {
        if (s == NULL) return -1;
        if (mutex_destroy(&(s->mutex_union.mutex)) == -1) return -1;
        while(s->queue != NULL){
            task_resume(s->queue);
        }
        s->active = 0;        
        return 0;
    }
#endif

/** ---------------------------------------------------------------------------------
 *                                      MUTEX FUNCTIONS
  ---------------------------------------------------------------------------------*/

int test_and_set (volatile int *lock)
{

    PPOS_PREEMPT_DISABLE;
    int old = *lock;
    *lock = 1;
    PPOS_PREEMPT_ENABLE;
    return old;
}

int mutex_create(mutex_t *m)
{
    if (m == NULL) return -1;
    m->lock = 0;
    m->active = 1;
    return 0;
}

int mutex_lock(mutex_t *m)
{
    if (m == NULL) return -1;
    if (m->active == 0) return -1;
    while (test_and_set(&(m->lock)))
    {
        if (m->active == 0) return -1; 
        task_yield();
    }
    return 0;
}
/*
int mutex_lock(mutex_t *m)
{
    if (m == NULL) return -1;
    if (m->active == 0) return -1;
    while (__atomic_test_and_set(&(m->lock), __ATOMIC_SEQ_CST))
    {
        if (m->active == 0) return -1; 
        task_yield();
    }
    return 0;
}
*/

int mutex_unlock(mutex_t *m)
{
    if (m == NULL) return -1;
    if (m->active == 0) return -1;
    m->lock = 0;
    return 0;
}

int mutex_destroy(mutex_t *m)
{
    if (m == NULL) return -1;
    m->active = 0;
    return 0;
}

queue_t* dequeue(queue_t **queue) {
    if (queue == NULL || *queue == NULL) {
        return NULL;
    }
    queue_t *queue_head = *queue;
    if (queue_head == queue_head->next)
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
/*
int queue_contains(queue_t **queue, int id)
{
    if (queue == NULL || *queue == NULL) {
        return 0;
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
	int i = 1;
	do
	{
		printf("Elemento(%d): %d \n",i, queue->id);
		queue = queue->next;
	}
	while(queue!=it);
} 
*/


void before_ppos_init () {
    // put your customization here
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
}

void after_ppos_init () {
    // put your customization here
#ifdef DEBUG
    printf("\ninit - AFTER");
#endif
    
}

void before_task_create (task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_create - BEFORE - [%d]", task->id);
#endif
}

void after_task_create (task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_create - AFTER - [%d]", task->id);
#endif
    
}

void before_task_exit () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_exit - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_exit () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_exit - AFTER- [%d]", taskExec->id);
#endif
    
}

void before_task_switch ( task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_switch - BEFORE - [%d -> %d]", taskExec->id, task->id);
#endif
    
}

void after_task_switch ( task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_switch - AFTER - [%d -> %d]", taskExec->id, task->id);
#endif
}

void before_task_yield () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - BEFORE - [%d]", taskExec->id);
#endif
}
void after_task_yield () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - AFTER - [%d]", taskExec->id);
#endif
}


void before_task_suspend( task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - BEFORE - [%d]", task->id);
#endif
}

void after_task_suspend( task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - AFTER - [%d]", task->id);
#endif
}

void before_task_resume(task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_resume - BEFORE - [%d]", task->id);
#endif
}

void after_task_resume(task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_resume - AFTER - [%d]", task->id);
#endif
}

void before_task_sleep () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_sleep () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - AFTER - [%d]", taskExec->id);
#endif
}

int before_task_join (task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_task_join (task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}


int before_sem_create (semaphore_t *s, int value) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_create (semaphore_t *s, int value) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_down (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_down (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_up (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_up (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_destroy (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_destroy (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_create (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_create (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_lock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_lock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_unlock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_unlock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_destroy (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_destroy (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_create (barrier_t *b, int N) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_create (barrier_t *b, int N) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_join (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_join (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_destroy (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_destroy (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_create (mqueue_t *queue, int max, int size) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_create (mqueue_t *queue, int max, int size) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_send (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_send (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_recv (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_recv (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_destroy (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_destroy (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_msgs (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_msgs (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}


