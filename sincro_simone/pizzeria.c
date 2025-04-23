#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 10  // Numero di piatti disponibili
#define M 4   // Numero di camerieri
#define K 6   // Numero massimo di clienti

// Variabili condivise
int num_pizze = 0;
int num_clienti = 0;

// Semafori
sem_t sem_CS;            // Mutex per la sezione critica
sem_t sem_pizze;         // Notifica che ci sono pizze disponibili
sem_t sem_spazi;         // Spazi liberi sul bancone per mettere le pizze
sem_t sem_cliente;       // Semaforo per il cliente (per gestire l'ingresso)

// Funzione per simulare la produzione di una pizza
void* pizzaiolo(void* arg) {
    while (1) {
        // Produzione della pizza (simulazione)
        sleep(1);

        // Sezione critica
        sem_wait(&sem_CS); 
        if (num_pizze < N) {
            num_pizze++;
            sem_post(&sem_pizze);  // Notifica che c'è una pizza pronta
        }
        sem_post(&sem_CS); 

        printf("Pizzaiolo ha messo una pizza. Totale pizze: %d\n", num_pizze);
    }
    return NULL;
}

// Funzione per simulare il cameriere
void* cameriere(void* arg) {
    while (1) {
        int pizze_da_portare;

        // Sezione critica per leggere num_clienti
        sem_wait(&sem_CS);
        if (num_clienti >= K)
            pizze_da_portare = 3;
        else
            pizze_da_portare = 2;
        sem_post(&sem_CS);

        // Attendere che ci siano abbastanza pizze
        for (int i = 0; i < pizze_da_portare; i++) {
            sem_wait(&sem_pizze);
        }

        // Sezione critica per modificare il numero di pizze
        sem_wait(&sem_CS);
        num_pizze -= pizze_da_portare;
        sem_post(&sem_CS);

        printf("Cameriere ha preso %d pizze. Rimangono: %d\n", pizze_da_portare, num_pizze);
        sleep(1);  // Simula il tempo di portare le pizze ai tavoli

        // Libera spazio per nuove pizze
        for (int i = 0; i < pizze_da_portare; i++) {
            sem_post(&sem_spazi);
        }
    }
    return NULL;
}

// Funzione per simulare il cliente
void* cliente(void* arg) {
    // Verifica se il locale è affollato
    sem_wait(&sem_CS);
    if (num_clienti >= K) {
        printf("Locale affollato! Numero di clienti: %d\n", num_clienti);
        sem_post(&sem_CS);
        return NULL; // Il cliente non entra, esce senza prendere una pizza
    }

    // Incrementa il numero di clienti
    num_clienti++;
    sem_post(&sem_CS);

    // Stampa se il locale è affollato o meno
    sem_wait(&sem_CS);
    if (num_clienti >= K) {
        printf("Locale affollato! Numero di clienti: %d\n", num_clienti);
    } else {
        printf("Locale non affollato. Numero di clienti: %d\n", num_clienti);
    }
    sem_post(&sem_CS);

    printf("Cliente ha preso una pizza.\n");
    sleep(2);  // Simula il tempo che il cliente mangia la pizza

    // Decrementa il numero di clienti
    sem_wait(&sem_CS);
    num_clienti--;
    sem_post(&sem_CS);

    return NULL;
}

int main() {
    // Inizializzazione semafori
    sem_init(&sem_CS, 0, 1);         // Mutex per la sezione critica
    sem_init(&sem_pizze, 0, 0);      // Nessuna pizza all'inizio
    sem_init(&sem_spazi, 0, N);      // Spazi liberi per i piatti (max N)
    sem_init(&sem_cliente, 0, 1);    // Semaforo per l'ingresso dei clienti

    // Crea i thread per pizzaioli
    pthread_t pizzaioli[2];          // Due pizzaioli
    for (int i = 0; i < 2; i++) {
        pthread_create(&pizzaioli[i], NULL, pizzaiolo, NULL);
    }

    // Crea i thread per camerieri
    pthread_t camerieri[M];          // M camerieri
    for (int i = 0; i < M; i++) {
        pthread_create(&camerieri[i], NULL, cameriere, NULL);
    }

    // Crea i thread per clienti
    pthread_t clienti[K];            // K clienti
    for (int i = 0; i < K; i++) {
        pthread_create(&clienti[i], NULL, cliente, NULL);
    }

    // Aspetta che tutti i thread finiscano (questo può essere opzionale per la simulazione continua)
    for (int i = 0; i < 2; i++) {
        pthread_join(pizzaioli[i], NULL);
    }
    for (int i = 0; i < M; i++) {
        pthread_join(camerieri[i], NULL);
    }
    for (int i = 0; i < K; i++) {
        pthread_join(clienti[i], NULL);
    }

    // Distruggi i semafori
    sem_destroy(&sem_CS);
    sem_destroy(&sem_pizze);
    sem_destroy(&sem_spazi);
    sem_destroy(&sem_cliente);

    return 0;
}
