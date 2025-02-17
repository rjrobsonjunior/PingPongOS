#include <signal.h>
#include <errno.h>
#include <string.h>
#include "ppos.h"
#include "ppos-core-globals.h"
#include "disk-driver.h"
#include "ppos-disk-manager.h"

// adicione todas as variaveis globais necessarias para implementar o gerenciado do disco
disk_t disk;

control_t ctr = {
    .signal_received = 0,
    .current_head_position =0,
    .blocks_traveled = 0
};

/* ------------------------------------ private functions --------------------------------------------------------*/

void diskDriverBody(void *args);

void diskSignalHandler();

diskrequest_t *schedule_fcfs(diskrequest_t *task_aux, int current_head_position);

diskrequest_t *schedule_sstf(diskrequest_t *task_aux, int current_head_position);

diskrequest_t *schedule_cscan(diskrequest_t *task_aux, int current_head_position);

/*----------------------------------------------------------------------------------------------------------------*/

// função para o tratamento de erros dos sinais - usada em disk_mgr_init()
void clean_exit_on_sig(int sig_num)
{
    printf ("\n ERROR[Signal = %d]: %d \"%s\"", sig_num, errno, strerror(errno));
    exit(errno);
}

/**
 * @brief função que inicializa toda a estrutura de controle do disco
 * @return retona 0 se foi bem sucedida a inicialização, do contrário, -1
 */
int disk_mgr_init (int *numBlocks, int *blockSize) {

    // coloque o codigo para inicializar o disco aqui

    /* Inicializa o disco*/
    if(disk_cmd (DISK_CMD_INIT, 0, 0) == -1)
    {
        return -1;
    } 

    /* Verifica se o número de blocos e o tamanho do bloco é válido*/
    if(*(numBlocks) = disk_cmd (DISK_CMD_DISKSIZE, 0, 0), *(numBlocks) == -1)
    {
        return -1;
    } 
    if(*(blockSize) = disk_cmd (DISK_CMD_BLOCKSIZE, 0, 0), *(blockSize) == -1)
    {
        return -1;
    } 

    /* cria o semaforo do acesso adico com um unico recurso*/
    if(sem_create(&(ctr.semaphore), 1) == -1)
    {
        return -1;
    } 

    disk.diskrequest_queue = NULL;

    ctr.disk_manager_task = (task_t *)malloc(sizeof(task_t));
    if (!ctr.disk_manager_task)
    {
        return -1; // Erro ao alocar memória para o gerente de disco
    }
    /*cria a tarefa que gerencia o disco*/
    task_create(ctr.disk_manager_task, diskDriverBody, NULL);

    /*A finalização de cada operação de leitura/escrita é indicada mais tarde pelo disco através de um sinal UNIX SIGUSR1*/
    signal(SIGUSR1, diskSignalHandler);

    // o seu codigo deve terminar ate aqui. 
    // As proximas linhas dessa função não devem ser modificadas
    signal(SIGSEGV, clean_exit_on_sig);

    return 0;
}

/**
 * @brief adiciona uma requisição de leitura ao disco no endereço passado pelo parametro block e copia o conteudo para buffer
 * @return retona 0 se foi bem sucedida a inicialização, do contrário, -1
 */
int disk_block_read(int block, void* buffer) {
    
    //obtém o semáforo de acesso ao disco
    if( sem_down(&(ctr.semaphore)) == -1)
    {
        return -1;
    }

    /* configura a nova requisição de tarfa*/
    diskrequest_t current_operation = {
        .block = block,
        .current_buffer = buffer,
        .is_write = 0,
        .next = NULL,
        .current_task = taskExec
    };
    
    //inclui o pedido na fila_disco
    queue_append((queue_t **)(&(disk.diskrequest_queue)), (queue_t*)(&current_operation));

    /*se o gerente de disco está dormindo,  acorda (põe ele na fila de prontas)*/
    if ((ctr.disk_manager_task->state) == PPOS_TASK_STATE_SUSPENDED)
    {
        task_resume(ctr.disk_manager_task);
    }

    // libera semáforo de acesso ao disco
    if( sem_up(&(ctr.semaphore)) == -1)
    {
        return -1;
    }

    task_yield();
    return 0;
}

/**
 * @brief adiciona uma requisição de escrita ao disco no endereço passado pelo parametro block e conteudo sendo o buffer
 * @return retona 0 se foi bem sucedida a inicialização, do contrário, -1
 */
int disk_block_write(int block, void* buffer) {
    
    //obtém o semáforo de acesso ao disco
    if( sem_down(&(ctr.semaphore)) == -1)
    {
        return -1;
    }

    /* configura a nova requisição de tarfa*/
    diskrequest_t current_operation = {
        .block = block,
        .current_buffer = buffer,
        .is_write = 1,
        .next = NULL,
        .current_task = taskExec
    };
    
   //inclui o pedido na fila_disco
    queue_append((queue_t **)(&(disk.diskrequest_queue)), (queue_t*)(&current_operation));

    /*se o gerente de disco está dormindo,  acorda (põe ele na fila de prontas)*/
    if ((ctr.disk_manager_task->state) == PPOS_TASK_STATE_SUSPENDED)
    {
       task_resume(ctr.disk_manager_task);
    }
    

    // libera semáforo de acesso ao disco
    if( sem_up(&(ctr.semaphore)) == -1)
    {
        return -1;
    }

    task_yield();

    return 0;
}


// Essa função implemeneta o escalonador de requisicoes de 
// leitura/scrita do disco usado pelo gerenciador do disco
// A função implementa a política FCFS.
diskrequest_t* disk_scheduler(diskrequest_t* queue) {
     // FCFS scheduler
    if ( queue != NULL ) {
        PPOS_PREEMPT_DISABLE
        diskrequest_t* request = queue;
        PPOS_PREEMPT_ENABLE
        return request;
    }
    return NULL;
}

/**
 * @brief A tarefa gerente de disco é responsável por tratar os pedidos de leitura/escrita das tarefas e os sinais 
 * gerados pelo disco. Ela é uma tarefa de sistema, similar ao dispatcher,
 * 
 * @param args 
 */
void diskDriverBody(void * args)
{
    int disk_status;
    diskrequest_t* next_request;
    while (1) 
    {
        //obtém o semáforo de acesso ao disco
        (void)sem_down(&(ctr.semaphore));

        if (readyQueue == taskMain)
        {
            readyQueue = readyQueue->next;
        }

        // se foi acordado devido a um sinal do disco
        if (ctr.signal_received == 1)
        {
            ctr.signal_received = 0;
            // acorda a tarefa cujo pedido foi atendido
            task_resume(disk.diskrequest_queue->current_task);
            (void)queue_remove((queue_t **)(&(disk.diskrequest_queue)), (queue_t *)((disk.diskrequest_queue)));
        }

        // se o disco estiver livre e houver pedidos de E/S na fila
        disk_status = disk_cmd(DISK_CMD_STATUS, 0, 0);
        if (disk_status == DISK_STATUS_IDLE && disk.diskrequest_queue != NULL)
        {
            // escolhe na fila o pedido a ser atendido, usando FCFS
            #ifdef FCFS
                
            #elif SSTF
            // solicita ao disco a operação de E/S, usando disk_cmd()
            #else
                next_request = disk.diskrequest_queue;    
                ctr.blocks_traveled += abs(disk.diskrequest_queue->block - ctr.current_head_position);    
            #endif
            if(next_request != NULL) 
            {
                if(next_request->is_write == 0)
                {
                    disk_cmd(DISK_CMD_READ, next_request->block, next_request->current_buffer);
                }
                else  
                {
                    disk_cmd(DISK_CMD_WRITE, next_request->block, next_request->current_buffer);
                }
                ctr.current_head_position = disk.diskrequest_queue->block;
                printf("Blocos percorridos: %d\n", ctr.blocks_traveled); // Exibe o número de blocos percorridos
            }
        }
        // libera semáforo de acesso ao disco
        (void)sem_up(&(ctr.semaphore));
        // suspende a tarefa corrente (retorna ao dispatcher)
        task_yield();
    }
}

/**
 * @brief função é chamada quando é gerada uma interrupação no disco após ter sido realizado uma requisição de tarefa
 * 
 */
void diskSignalHandler()
{
    (void)sem_down(&(ctr.semaphore));
    ctr.signal_received = 1;
    (void)sem_up(&(ctr.semaphore));
    task_resume(ctr.disk_manager_task); // Acorda a tarefa gerente de disco
}