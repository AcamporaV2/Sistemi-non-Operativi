#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define N 5
#define M 10

int buffer[N];
int numeri[N];
int buffer_pronto = 0;

int max = -1;
int vincitore = -1;
int id_consumatore = -1;

sem_t mutex;
sem_t semafori[N];
sem_t consumato;

void* lavoratore(void* arg) {
    int id = *(int*)arg;
    while (1) {
        sem_wait(&semafori[id]);

        sem_wait(&mutex);
        if (buffer_pronto == 1 && id == vincitore) {
            printf("🛠️  [Lavoratore %d] Consuma valore: %d\n", id, buffer[id]);

            buffer[id] = 0;
            buffer_pronto = 0;
            id_consumatore = id;

            sem_post(&consumato);
        }
        sem_post(&mutex);
    }
    return NULL;
}

void* coordinatore(void* arg) {
    while (1) {
        sem_wait(&mutex);

        while (buffer_pronto != 0) {
            sem_post(&mutex);
            usleep(1000);  // Sleep breve per evitare busy wait aggressivo
            sem_wait(&mutex);
        }

        // Produzione
        printf("🏭 [Coordinatore] Produce: ");
        for (int i = 0; i < N; i++) {
            buffer[i] = rand() % 100;
            printf("%d ", buffer[i]);
        }
        printf("\n");
        buffer_pronto = 1;

        // Assegna numeri casuali
        printf("🎲 [Coordinatore] Assegna numeri: ");
        for (int i = 0; i < N; i++) {
            numeri[i] = rand() % (M + 1);
            printf("%d ", numeri[i]);
        }
        printf("\n");

        // Trova il vincitore
        max = -1;
        vincitore = -1;
        for (int i = 0; i < N; i++) {
            if (numeri[i] > max) {
                max = numeri[i];
                vincitore = i;
            }
        }

        printf("🏆 [Coordinatore] Processo vincitore: %d (numero: %d)\n", vincitore, max);

        sem_post(&semafori[vincitore]);
        sem_post(&mutex);

        sem_wait(&consumato);
        printf("✅ [Coordinatore] Lavoratore %d ha consumato il buffer\n\n", id_consumatore);

        sleep(1);  // Per rendere l'output leggibile
    }

    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t tid_coordinatore;
    pthread_t tid_lavoratori[N];
    int ids[N];

    // Inizializzazione semafori
    sem_init(&mutex, 0, 1);
    sem_init(&consumato, 0, 0);
    for (int i = 0; i < N; i++) {
        sem_init(&semafori[i], 0, 0);
    }

    // Creazione thread lavoratori
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&tid_lavoratori[i], NULL, lavoratore, &ids[i]);
    }

    // Creazione thread coordinatore
    pthread_create(&tid_coordinatore, NULL, coordinatore, NULL);

    // Join non necessari perché il programma gira all'infinito
    pthread_join(tid_coordinatore, NULL);
    return 0;
}
