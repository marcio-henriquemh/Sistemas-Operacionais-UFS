#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#define MAXIMO_DE_CLIENTES 10



bool aberto = true;

typedef struct Cliente {
    int identificador;
    struct Cliente* proximo;
} Cliente;


Cliente* inicio = NULL;
Cliente* fim = NULL;

int total_clientes = 0;
int proximo_identificador = 1;

pthread_mutex_t mutex_fila = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_clientes;

void adicionar_cliente(int identificador) {
    Cliente* novo = malloc(sizeof(Cliente));
    novo->identificador = identificador;
    novo->proximo = NULL;

    if (fim == NULL) {
        inicio = fim = novo;
    } else {
        fim->proximo = novo;
        fim = novo;
    }

    total_clientes++;
}

int remover_cliente() {
    if (inicio == NULL) return -1;

    Cliente* temp = inicio;
    int id = temp->identificador;
    inicio = inicio->proximo;
    if (inicio == NULL) fim = NULL;
    free(temp);

    total_clientes--;
    return id; // Corrigido: retorna o id do cliente removido
}


void* barbeiro(void* arg) {
    char* nome = (char*)arg;

    while (aberto) {
        sem_wait(&sem_clientes); // espera cliente

        pthread_mutex_lock(&mutex_fila);
        int cliente_id = remover_cliente();
        pthread_mutex_unlock(&mutex_fila);

        if (cliente_id != -1) {
            printf("💈 %s começou a cortar o cabelo do Cliente %d\n", nome, cliente_id);
            fflush(stdout);
            int tempo_corte = rand() % 6 + 5; // entre 5 e 10s
            sleep(tempo_corte);
            printf("✅ %s terminou o corte do Cliente %d em %ds\n", nome, cliente_id, tempo_corte);
            fflush(stdout);
        }
    }

    return NULL;
}

void* chegada_clientes(void* arg) {
    while (aberto) {
        sleep(rand() % 3 + 4); // entre 4 e 6s

        pthread_mutex_lock(&mutex_fila);

        if (total_clientes >= MAXIMO_DE_CLIENTES) {
            printf("🚫 Cliente %d foi embora — Fila cheia!\n", proximo_identificador);
            fflush(stdout);
        } else {
            printf("👤 Cliente %d entrou na fila\n", proximo_identificador);
            fflush(stdout);
            adicionar_cliente(proximo_identificador++);
            sem_post(&sem_clientes); // sinaliza barbeiro
        }

        printf("📊 Clientes na fila agora: %d\n", total_clientes);
        fflush(stdout);

        pthread_mutex_unlock(&mutex_fila);
    }

    return NULL;
}


void* temporizador(void* arg) {
    sleep(15); // simula 60 segundos abertos
    aberto = false;
    // libera todos os barbeiros que estejam esperando
    sem_post(&sem_clientes);
    sem_post(&sem_clientes);
    return NULL;
}

int main() {
    srand(time(NULL));
    sem_init(&sem_clientes, 0, 0);


    pthread_t t_barbeiro1, t_barbeiro2, t_chegada,t_teporizador;

    pthread_t t_temporizador;
    pthread_create(&t_temporizador, NULL, temporizador, NULL);

    pthread_create(&t_barbeiro1, NULL, barbeiro, "Barbeiro 1");
    pthread_create(&t_barbeiro2, NULL, barbeiro, "Barbeiro 2");
    pthread_create(&t_chegada, NULL, chegada_clientes, NULL);

    pthread_join(t_barbeiro1, NULL);
    pthread_join(t_barbeiro2, NULL);
    pthread_join(t_chegada, NULL);


    
    return 0;
}
