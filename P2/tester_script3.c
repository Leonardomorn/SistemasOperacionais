// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste da gestão básica de tarefas

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define MAXTASK 3

task_t task ;

// corpo das threads
void BodyTask (void * arg)
{
   printf ("Estou na tarefa %5d\n\n\n\n", task_id()) ;
   printf("Saí da tarefa ");
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   printf("A tarefa main %5d tem esse numero\n\n", task_id());
   int i ;

   printf ("main: inicio\n");

   ppos_init () ;

   // inicia MAXTASK tarefas, ativando cada uma apos sua criacao
   for (i=0; i<MAXTASK; i++)
   {
     task_init (&task, BodyTask, NULL) ;
     task_switch (&task) ;
     printf("Estou na tarefa %5d que é a main \n", task_id());
   }

   printf ("main: fim\n");

   task_exit (0);
}
