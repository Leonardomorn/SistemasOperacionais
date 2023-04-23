#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "dispatcher.h"




// #define DEBUG 1
#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

task_t *currentTask_global;
task_t dispatcherTask_global;
task_t mainTask_global;
int idCounter_global, userTasks_global;









//PARTE DA FILA DE TASKS - INICIO

#define QUEUESIZE 31000

// A estrutura "taskqueue_t" será usada com as funções de queue.c usando um
// casting para o tipo "queue_t". Isso funciona bem, se os campos iniciais
// de ambas as estruturas forem os mesmos. De acordo com a seção 6.7.2.1 do
// padrão C99: "Within a structure object, the non-bit-ﬁeld members and the
// units in which bit-ﬁelds reside have addresses that increase in the order
// in which they are declared.".

typedef struct readyTaskQueue_t
{
   struct readyTaskQueue_t *prev ;  // ptr para usar cast com queue_t
   struct readyTaskQueue_t *next ;  // ptr para usar cast com queue_t
   //int id ;
   // outros campos podem ser acrescidos aqui
} readyTaskQueue_t ;

readyTaskQueue_t item[QUEUESIZE];
readyTaskQueue_t *ready_task_queue;
int ret ;


//PARTE DA FILA DE TASKS - FINAL









void ppos_init ()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);
    idCounter_global = 0;
    userTasks_global = 0;

    #ifdef DEBUG
    printf ("Settando current task em main\n") ;
    #endif
    //coloca a main como atual
    currentTask_global = &mainTask_global;

    //inicia a tarefa dispatcher
    task_init(&dispatcherTask_global, dispatcher, NULL);

    #ifdef DEBUG
    printf ("Terminada inicializacao de sistema\n");
    #endif


}


// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
int task_init (task_t *task,			// descritor da nova tarefa
               void  (*start_func)(void *),	// funcao corpo da tarefa
               void   *arg) 			// argumentos para a tarefa
{
    char *stack ;
    #ifdef DEBUG
    printf ("criando um contexto\n") ;
    #endif
    #ifdef DEBUG
    printf ("obtendo o contexto\n") ;
    #endif
    getcontext (&(task->context)) ;

    //alocando a pilha
    stack = malloc (STACKSIZE) ;
    if (stack)
    {
        #ifdef DEBUG
        printf ("determinando os descritores do contexto\n") ;
        #endif
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
   }

    #ifdef DEBUG
    printf ("modificando os valores do contexto\n") ;
    printf("salvando a funcao do ponteiro%p \n", (void*)(start_func));
    #endif
    if(arg)
        makecontext (&(task->context), (void*)(start_func),1, arg) ;
    else
        makecontext (&(task->context), (void*)(start_func),0) ;

    idCounter_global++;
    #ifdef DEBUG
    printf ("atribuindo um ID %d\n", idCounter_global) ;
    #endif
    task->id = idCounter_global;

    if(idCounter_global > 1) //0 e 1 sao reservados para main e dispatcher
    {
        //adiciona a tarefa para a fila
        queue_append((queue_t **) &ready_task_queue, (queue_t *) &task);
        userTasks_global++;
    }

    #ifdef DEBUG
    printf ("criação finalizada\n") ;
    #endif




    return task->id; 
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id ()
{
    return currentTask_global->id;
}

// Termina a tarefa corrente com um valor de status de encerramento e retorna para a main
void task_exit (int exit_code) 
{
    if(currentTask_global == &mainTask_global)
    {
        perror("task exit in main");
        exit(1);
    }

    if(currentTask_global == &dispatcherTask_global)
    {
        exit(1);
    }
    task_switch(&mainTask_global);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{
    #ifdef DEBUG
    printf ("solicitando troca de contexto\n") ;
    #endif
    if (task)
    {
    task_t *aux = currentTask_global;
    currentTask_global = task;


    swapcontext(&(aux->context), &(currentTask_global->context));
    return 0;
    }
    return -1;
}


