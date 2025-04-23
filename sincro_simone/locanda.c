#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define N 5       // capacità del calderone
#define K 2       // numero di cuochi
#define M 3       // numero di avventori

// Buffer circolare
int Zuppe[N];
int prod_ptr = 0;
int cons_ptr = 0;
int max_zuppe = 3;

// Mutex e variabili di condizione
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t slot_libero = PTHREAD_COND_INITIALIZER;
pthread_cond_t zuppa_disponibile = PTHREAD_COND_INITIALIZER;

void* cuoco(void* arg) {
    int id = *(int*)arg;
    while (1) {
        // Simula preparazione

        sleep(rand() % 1 + 1);  //Ipoteticamente se togliamo lo sleep qui produce come un cavallo e può finire nel while
        
        printf("Cuoco %d ha preparato una porzione.\n", id);

        pthread_mutex_lock(&mtx);

        // Se buffer pieno, aspetto
        while (max_zuppe == N) {
            printf("Cuoco %d waiting on slot_libero (calderone pieno)...\n", id);
            pthread_cond_wait(&slot_libero, &mtx);
            printf("Cuoco %d resumed from slot_libero.\n", id);
        }

        // Inserisco la zuppa
        Zuppe[prod_ptr] = 1;
        prod_ptr = (prod_ptr + 1) % N;
        max_zuppe++;
        printf("Cuoco %d ha aggiunto una porzione. Totale = %d\n", id, max_zuppe);

        // Segnalo agli avventori
        printf("Cuoco %d signaling zuppa_disponibile.\n", id);
        pthread_cond_signal(&zuppa_disponibile);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

void* avventore(void* arg) {
    int id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&mtx);

        // Se buffer vuoto, aspetto
        while (max_zuppe == 0) {
            printf("Avventore %d waiting on zuppa_disponibile (calderone vuoto)...\n", id);
            pthread_cond_wait(&zuppa_disponibile, &mtx);
            printf("Avventore %d resumed from zuppa_disponibile.\n", id);
        }

        // Rimuovo la zuppa
        Zuppe[cons_ptr] = 0;
        cons_ptr = (cons_ptr + 1) % N;
        max_zuppe--;
        printf("Avventore %d ha preso una porzione. Rimaste = %d\n", id, max_zuppe);

        // Segnalo ai cuochi
        printf("Avventore %d signaling slot_libero.\n", id);
        pthread_cond_signal(&slot_libero);
        pthread_mutex_unlock(&mtx);

        // Simulo il tempo di consumo
        sleep(rand() % 2 + 1);
        printf("Avventore %d ha gustato la zuppa.\n", id);
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t thread_cuochi[K];
    pthread_t thread_avventori[M];
    int ids_cuochi[K], ids_avventori[M];

    // Creo i thread cuochi
    for (int i = 0; i < K; i++) {
        ids_cuochi[i] = i + 1;
        if (pthread_create(&thread_cuochi[i], NULL, cuoco, &ids_cuochi[i]) != 0) {
            perror("pthread_create cuoco");
            exit(1);
        }
    }

    // Creo i thread avventori
    for (int i = 0; i < M; i++) {
        ids_avventori[i] = i + 1;
        if (pthread_create(&thread_avventori[i], NULL, avventore, &ids_avventori[i]) != 0) {
            perror("pthread_create avventore");
            exit(1);
        }
    }

    // Aspetto che i thread girino (non terminano mai)
    for (int i = 0; i < K; i++) {
        pthread_join(thread_cuochi[i], NULL);
    }
    for (int i = 0; i < M; i++) {
        pthread_join(thread_avventori[i], NULL);
    }

    return 0;
}
