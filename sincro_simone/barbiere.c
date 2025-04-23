#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_CLIENTI 10
#define SEDIE_ATTESA 3

sem_t clienti;
sem_t barbiere;
pthread_mutex_t accessoSedie;

int sedie_libere = SEDIE_ATTESA;

void* funzione_barbiere(void* arg) {
    while (1) {
        sem_wait(&clienti); // Aspetta un cliente
        pthread_mutex_lock(&accessoSedie);
        sedie_libere++; // Libera una sedia
        pthread_mutex_unlock(&accessoSedie);

        sem_post(&barbiere); // Il barbiere è pronto
        printf("💈 Il barbiere sta tagliando i capelli...\n");
        sleep(3); // Simula il taglio
        printf("✅ Il barbiere ha finito.\n");
    }
    return NULL;
}

void* funzione_cliente(void* arg) {
    int id = *(int*)arg;
    free(arg);

    pthread_mutex_lock(&accessoSedie);
    if (sedie_libere > 0) {
        sedie_libere--;
        printf("👤 Cliente %d si siede. Sedie libere: %d\n", id, sedie_libere);
        pthread_mutex_unlock(&accessoSedie);

        sem_post(&clienti); // Notifica il barbiere
        sem_wait(&barbiere); // Aspetta il taglio
        printf("💇 Cliente %d sta ricevendo il taglio.\n", id);
    } else {
        pthread_mutex_unlock(&accessoSedie);
        printf("🚶 Cliente %d se ne va: sala piena.\n", id);
    }
    return NULL;
}

int main() {
    pthread_t th_barbiere;
    pthread_t th_clienti[MAX_CLIENTI];

    sem_init(&clienti, 0, 0);
    sem_init(&barbiere, 0, 0);
    pthread_mutex_init(&accessoSedie, NULL);

    pthread_create(&th_barbiere, NULL, funzione_barbiere, NULL);

    for (int i = 0; i < MAX_CLIENTI; i++) {
        sleep(rand() % 2); // Arrivo casuale dei clienti
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&th_clienti[i], NULL, funzione_cliente, id);
    }

    for (int i = 0; i < MAX_CLIENTI; i++) {
        pthread_join(th_clienti[i], NULL);
    }

    // In un'app reale: codice per terminare il barbiere pulitamente

    return 0;
}
