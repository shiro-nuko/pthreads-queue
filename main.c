#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define TAM_FILA   11      // Tamanho da fila (tem de somar 1 no tamanho
                           // desejado pois uma posi��o do vetor nunca � usada.

int pacotes[TAM_FILA];     // Janela de pacotes

unsigned send_base = 0,
         next_seq_num = 0; // Ponteiros de retirada e inser��o na janela

pthread_mutex_t mutex;
pthread_cond_t cond_not_full;
pthread_cond_t cond_freed_space;

//*************************************
// Retorna o total de elementos na fila
//*************************************
int tamFila(void)
{
  if(next_seq_num >= send_base)
     return next_seq_num - send_base;
  else
     return (TAM_FILA-send_base)+next_seq_num;
}

//*********************************************************************
// Insere um valor na fila. Essa rotina s� deve ser chamada se houver
// espa�o livre na janela pois ela n�o testa situa��o de overflow.
//*********************************************************************
static void insereFila(int valor)
{
  printf("Queue Size Before Producing: %d\n", tamFila());
  pthread_mutex_lock(&mutex);
  while(tamFila() == TAM_FILA - 1){
    printf("Waiting for queue to be freed\n");
    pthread_cond_wait(&cond_freed_space, &mutex);
  }
  pacotes[next_seq_num] = valor;
  printf("Produced value %d\n", valor);
  next_seq_num = ++next_seq_num % TAM_FILA; // Se chegar no final da fila,
                                            // volta a ser 0
  pthread_mutex_unlock(&mutex);
  pthread_cond_broadcast(&cond_not_full);
  printf("Queue Size After Producing: %d\n", tamFila());
}

//************************************************************
// Desenfileira e retorna o valor mais antigo da fila.
// Essa rotina s� deve ser chamada se tiver pacotes na fila
// pois ela n�o testa se fila est� vazia.
//************************************************************
static int retiraFila(void)
{
  printf("Queue Size Before Consuming: %d\n", tamFila());
  pthread_mutex_lock(&mutex);
  while(next_seq_num == send_base){
    printf("Waiting for products\n");
    pthread_cond_wait(&cond_not_full, &mutex);
  }
  int valor = pacotes[send_base];

  printf("Consumed value %d!\n", valor);
  send_base = ++send_base % TAM_FILA; // Qdo chega no final da fila, volta a 0

  pthread_mutex_unlock(&mutex);
  pthread_cond_broadcast(&cond_freed_space);
  printf("Queue Size After Consuming: %d\n", tamFila());
  return valor;
}

void* producer(void* arg){
  while(1){
    int value = (rand() % 1000);
    if((rand() % 100) % 2 == 0){
      insereFila(value);
    }
  }
}

void* consumer(void* arg){
  while(1){
    if((rand() % 100) % 2 != 0){
      retiraFila();
    }
  }
}

int main(int argc, char* argv[])
{
  srand(time(NULL));

  int producers = atoi(argv[1]);
  int consumers = atoi(argv[2]);
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_freed_space, NULL);
  pthread_cond_init(&cond_not_full, NULL);

  pthread_t producers_thread[producers];
  printf("================STARTING PRODUCER INITIALIZATION PROCESS================\n\n");
  for(int i = 0; i < producers; i++){
    printf("Producer Thread #%d Created! Returned %d\n", i, pthread_create(&producers_thread[i], NULL, producer, NULL));
  }

  printf("\n================FINISHED INITIALIZING PRODUCERS================\n");
  printf("\n================STARTING CONSUMER INITIALIZATION PROCESS================\n\n");

  pthread_t consumers_thread[producers];
  for(int i = 0; i < consumers; i++){
    printf("Consumer Thread #%d Created! Returned %d\n", i, pthread_create(&consumers_thread[i], NULL, consumer, NULL));
  }
  printf("\n================FINISHED INITIALIZING CONSUMERS================\n");

  for(int i = 0; i < producers; i++){
    printf("Producer Thread #%d Joined! Returned %d\n", i, pthread_join(producers_thread[i], NULL));
  }

  for(int i = 0; i < consumers; i++){
    printf("Consumer Thread #%d Joined! Returned %d\n", i, pthread_join(consumers_thread[i], NULL));
  }

  return 0;
}
