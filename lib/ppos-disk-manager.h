// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

//#define DEBUG_DISK 1

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// structura de dados que representa um pedido de leitura/escrita ao disco
typedef struct diskrequest_t {
    struct diskrequest_t* next;  // pre-requisito para usar a biblioteca queue.h
    struct diskrequest_t* prev;  // pre-requisito para usar a biblioteca queue.h

    // inserir os campos adicionais a partir daqui... 
    task_t *current_task;               /* ponteiro para a tarefa*/
    char is_write;                      /* é escrita (1) é leitura(0)*/
    int block;                          /* endereço do bloco*/
    void *current_buffer;               /* endereço para o conteudo que deve ser lido ou escrito*/

} diskrequest_t;

// estrutura que representa um disco no sistema operacional
// structura de dados que representa o disco para o SO
typedef struct {
    // inserir os campos adicionais a partir daqui...
    diskrequest_t *diskrequest_queue;   /* fila de requisiçãoao disco*/
} disk_t;

typedef struct
{
    int n_blocks;                      /* número de blocos*/ 
    int block_size;                    /* tamanho do bloco em bytes*/
    semaphore_t semaphore;             /* semaforo que control a o acesso ao disco*/
    task_t *disk_manager_task;          /* tarefa que gerencia o disco*/
    int current_head_position;         /* Posição atual do cabeçote do disco*/
    int blocks_traveled;               /* Número de blocos percorridos*/
    char signal_received;              /* sinal do disco recebido*/
}control_t;


// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

// escalonador de requisições do disco
diskrequest_t* disk_scheduler();

#endif