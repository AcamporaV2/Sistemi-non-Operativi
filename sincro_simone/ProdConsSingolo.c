#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int buffer[1];                 // Buffer come array di un solo elemento

pthread_mutex_t mutex;
sem_t semVuoto, semPieno;

void* produttore(void* arg) {
    for (int i = 1; i <= 5; i++) {
        sleep(rand() % 2);
        sem_wait(&semVuoto); // Aspetta che il buffer sia vuoto
        pthread_mutex_lock(&mutex);

        buffer[0] = i;
        printf("📦 Produttore ha prodotto: %d\n", buffer[0]);

        pthread_mutex_unlock(&mutex);
        sem_post(&semPieno); // Segnala che il buffer è pieno
    }
    return NULL;
}

void* consumatore(void* arg) {
    for (int i = 1; i <= 5; i++) {
        sem_wait(&semPieno); // Aspetta che il buffer sia pieno
        pthread_mutex_lock(&mutex);

        printf("🍽️ Consumatore ha consumato: %d\n", buffer[0]);

        pthread_mutex_unlock(&mutex);
        sem_post(&semVuoto); // Segnala che il buffer è vuoto
        sleep(rand() % 2);
    }
    return NULL;
}

int main() {
    pthread_t thProd, thCons;

    pthread_mutex_init(&mutex, NULL);
    sem_init(&semVuoto, 0, 1);  // Inizialmente vuoto
    sem_init(&semPieno, 0, 0);  // Inizialmente non pieno

    pthread_create(&thProd, NULL, produttore, NULL);
    pthread_create(&thCons, NULL, consumatore, NULL);

    pthread_join(thProd, NULL);
    pthread_join(thCons, NULL);

    pthread_mutex_destroy(&mutex);
    sem_destroy(&semVuoto);
    sem_destroy(&semPieno);

    return 0;
}
