#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5   // Dimensione del buffer
#define NUM_ELEMENTI 10

int buffer[N];
int in = 0;
int out = 0;

pthread_mutex_t mutex;
sem_t semVuoto, semPieno;

void* produttore(void* arg) {
    for (int i = 1; i <= NUM_ELEMENTI; i++) {
        sleep(rand() % 2);
        sem_wait(&semVuoto);              // Aspetta uno slot libero
        pthread_mutex_lock(&mutex);

        buffer[in] = i;
        printf("📦 Produttore ha inserito: %d in buffer[%d]\n", i, in);
        in = (in + 1) % N;

        pthread_mutex_unlock(&mutex);
        sem_post(&semPieno);              // Segnala un elemento pieno
    }
    return NULL;
}

void* consumatore(void* arg) {
    for (int i = 1; i <= NUM_ELEMENTI; i++) {
        sem_wait(&semPieno);              // Aspetta un elemento disponibile
        pthread_mutex_lock(&mutex);

        int val = buffer[out];
        printf("🍽️ Consumatore ha consumato: %d da buffer[%d]\n", val, out);
        out = (out + 1) % N;

        pthread_mutex_unlock(&mutex);
        sem_post(&semVuoto);              // Segnala uno slot vuoto
        sleep(rand() % 2);
    }
    return NULL;
}

int main() {
    pthread_t thProd, thCons;

    pthread_mutex_init(&mutex, NULL);
    sem_init(&semVuoto, 0, N);  // All'inizio tutti vuoti
    sem_init(&semPieno, 0, 0);  // Nessun pieno

    pthread_create(&thProd, NULL, produttore, NULL);
    pthread_create(&thCons, NULL, consumatore, NULL);

    pthread_join(thProd, NULL);
    pthread_join(thCons, NULL);

    pthread_mutex_destroy(&mutex);
    sem_destroy(&semVuoto);
    sem_destroy(&semPieno);

    return 0;
}
