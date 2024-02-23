#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define SIZE 10 //tamanho máximo da fila
#define THREAD_NUM 3 //número de pares de threads (nesse caso, 3 produtoras e 3 consumidoras)

//Compile:  gcc -g -Wall -o parte2 parte2.c -lpthread -lrt
//Usage:    ./parte2

typedef struct Relogio { 
   int p[3];
} Relogio;

Relogio filaRelogio[SIZE];
int contador = 0;


pthread_mutex_t mutex; 
pthread_cond_t condCheio;
pthread_cond_t condVazio;

int tempoProd; 
int tempoCons; 

void printRelogio(Relogio *relogio, int processo) {
   printf("Thread: %d, Relogio consumido: (%d, %d, %d)\n", processo, relogio->p[0], relogio->p[1], relogio->p[2]);
}

void produzirRelogio(int threadId) {
    //cria o Relogio
    Relogio *relogio = (Relogio*) malloc(sizeof(Relogio));
    relogio->p[0] = rand() % 10000;
    relogio->p[1] = rand() % 10000;
    relogio->p[2] = rand() % 10000;
    
    pthread_mutex_lock(&mutex); 
    //regiao critica
    while  (contador == SIZE) { 
        printf("Fila cheia! Thread %d não pode produzir. \n", threadId);
        pthread_cond_wait(&condCheio, &mutex);
    }
    
    filaRelogio [contador] = *relogio; //insere no final da fila
    contador++;
    printf("Thread: %d, Relogio produzido: (%d, %d, %d)\n", threadId, relogio->p[0], relogio->p[1], relogio->p[2]);
    
    pthread_mutex_unlock(&mutex); //desbloqueia acesso a região critica
    pthread_cond_signal(&condVazio); //envia sinal que a fila não está vazia
    
    free(relogio);
}

void consumirRelogio(int threadId) {
    pthread_mutex_lock(&mutex);
    
    while  (contador == 0) { 
        printf("Fila vazia! Thread %d não pode consumir. \n", threadId);
        pthread_cond_wait(&condVazio, &mutex);
    }
    
    Relogio relogio = filaRelogio[0];
    printRelogio(&relogio, threadId);
    for (int i = 0; i < contador-1; i++) { 
        filaRelogio[i] = filaRelogio[i+1];
    }
    contador--;
    
    pthread_mutex_unlock(&mutex); 
    pthread_cond_signal(&condCheio); 
    
}

void filaCheia() {
    tempoCons = 3;
    tempoProd = 1;
}


void filaVazia() {
    tempoCons = 1;
    tempoProd = 3;
}


void *threadProdutora(void* arg) {
    long id = (long) arg; 
    while (1){ 
        produzirRelogio(id);
        sleep(tempoProd* (rand() %4));
   } 
   return NULL;
} 

void *threadConsumidora(void* arg) {
    long id = (long) arg; 
    while(1) {
        consumirRelogio(id);  
        sleep(tempoCons*(rand() %4));
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condVazio, NULL);
    pthread_cond_init(&condCheio, NULL);
    
    pthread_t thread[THREAD_NUM*2]; 
    
    filaCheia();
    
    
    
    long i;
    for (i = 0; i < THREAD_NUM; i++){  
        if (pthread_create(&thread[i], NULL, &threadConsumidora, (void*) i) != 0) { //cria threads consumidoras
            perror("Falha ao criar a Thread");
        }  
    }
    for (i = THREAD_NUM; i < THREAD_NUM*2; i++){  
        if (pthread_create(&thread[i], NULL, &threadProdutora, (void*) i) != 0) { //cria threads produtoras
            perror("Falha ao criar a Thread");
        }  
    }
    
    for (i = 0; i < THREAD_NUM*2; i++){  
        if (pthread_join(thread[i], NULL) != 0) { //join das threads 
        perror("Falha ao entrar na Thread");
        }  
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condVazio);
    pthread_cond_destroy(&condCheio);
    return 0;
}

