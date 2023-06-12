// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#define READY      0
#define TERMINATED 1
#define SUSPENDED  2
#define USER_TASK 10
#define SYSTEM_TASK 11

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h" //biblioteca de filas genéricas

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  short priority_static;
  short priority_dynamic;     //prioridade da tarefa a ser executada, -20 é prioridade máxima e 20 prioridade mínima
  short type; // tarefas podem ser do tipo sistema (exemplo: dispatcher) ou usuário (exemplo: taferas regulares)
  short quantum_ticks; // tarefas tem um máximo de ticks a cada quantum antes de serem retiradas por preempção
  unsigned int initial_time;
  unsigned int final_time;
  unsigned int execution_time;
  unsigned int initial_time_processor;
  unsigned int final_time_processor;
  unsigned int processor_time;
  unsigned int activations;
  struct task_t *waiting_this_task_queue;
  int exitCode;
  unsigned int wake_up_time; // ao acionar o sleep, a tarefa acordará no tempo wake_up_time
  int preemption_blocker;

  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif


void dispatcher ();
//define quem é a próxima tarefa a ser executada
task_t * scheduler ();
//devolve a tarefa mínima - a de maior prioridade
task_t* first_min();
//aumenta a prioridade de uma tarefa
void raise_priority(task_t* to_be_executed_task);
//volta a prioridade estática para a tarefa
void restore_dynamic__into_static_prio(task_t *taskAux);
//define se a tarefa é tipo sistema ou usuário
void task_set_type(task_t* task, short type);
void initialize_handler();
void initialize_timer();
void handling (int signum);
unsigned int systime();
void initializeMainTask();
int task_wait(task_t *task);
void task_resume (task_t *task, task_t **queue);
void task_suspend (task_t **queue);
void free_suspended_queue();
void task_sleep (int t);
void wake_sleeping_tasks();
