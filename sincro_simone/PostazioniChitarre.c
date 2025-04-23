#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// Costanti
#define M 5    // Numero di postazioni di prova
#define N 10   // Numero di modelli di chitarre
#define C 2    // Numero di chitarre per ogni modello
#define NUM_CLIENTI 20  // Numero totale di clienti

// Variabili condivise
int chitarre_disponibili[N];    // Array che indica quante chitarre di ogni modello sono disponibili
sem_t sem_mutex;                // Per la mutua esclusione
sem_t sem_postazioni;           // Semaforo per le postazioni disponibili
sem_t sem_chitarre[N];          // Semafori per ogni modello di chitarra

// Funzione per simulare l'utilizzo della chitarra
void usa_chitarra(int id_cliente, int modello) {
    printf("Cliente %d sta suonando la chitarra modello %d\n", id_cliente, modello);
    // Simula il tempo di utilizzo della chitarra (tra 1 e 3 secondi)
    sleep(rand() % 3 + 1);
}

// Funzione eseguita dal thread cliente
void* cliente(void* arg) {
    int id = *((int*)arg);
    free(arg); // Libera la memoria allocata per l'id
    
    // Il cliente sceglie un modello
    int modello_richiesto;
    
    sem_wait(&sem_mutex);
    modello_richiesto = rand() % N;
    sem_post(&sem_mutex);
    
    printf("Cliente %d vuole provare il modello %d\n", id, modello_richiesto);
    
    // Richiedi una postazione
    printf("Cliente %d è in attesa di una postazione\n", id);
    sem_wait(&sem_postazioni);
    printf("Cliente %d ha ottenuto una postazione\n", id);
    
    // Richiedi una chitarra del modello desiderato
    printf("Cliente %d è in attesa di una chitarra modello %d\n", id, modello_richiesto);
    sem_wait(&sem_chitarre[modello_richiesto]);
    
    // Sezione critica per aggiornare lo stato
    sem_wait(&sem_mutex);
    chitarre_disponibili[modello_richiesto]--;
    printf("Cliente %d ha preso una chitarra modello %d (rimaste: %d)\n", 
           id, modello_richiesto, chitarre_disponibili[modello_richiesto]);
    sem_post(&sem_mutex);
    
    // Usa la chitarra
    usa_chitarra(id, modello_richiesto);
    
    // Rilascia la chitarra
    sem_wait(&sem_mutex);
    chitarre_disponibili[modello_richiesto]++;
    printf("Cliente %d ha restituito la chitarra modello %d (disponibili: %d)\n", 
           id, modello_richiesto, chitarre_disponibili[modello_richiesto]);
    sem_post(&sem_mutex);
    
    // Rilascia le risorse
    sem_post(&sem_chitarre[modello_richiesto]);
    sem_post(&sem_postazioni);
    
    printf("Cliente %d ha terminato\n", id);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_CLIENTI];
    
    // Inizializza il generatore di numeri casuali
    srand(time(NULL));
    
    // Inizializza i semafori
    sem_init(&sem_mutex, 0, 1);
    sem_init(&sem_postazioni, 0, M);
    
    // Inizializza l'array delle chitarre disponibili e i relativi semafori
    for (int i = 0; i < N; i++) {
        chitarre_disponibili[i] = C;
        sem_init(&sem_chitarre[i], 0, C);
    }
    
    printf("Simulazione negozio di chitarre avviata\n");
    printf("Postazioni: %d, Modelli: %d, Chitarre per modello: %d\n", M, N, C);
    
    // Crea i thread per i clienti
    for (int i = 0; i < NUM_CLIENTI; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads[i], NULL, cliente, id);
        
        // Piccolo ritardo tra la creazione di ogni cliente
        usleep(rand() % 500000); // 0-500ms
    }
    
    // Attendi il completamento di tutti i thread
    for (int i = 0; i < NUM_CLIENTI; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Distruggi i semafori
    sem_destroy(&sem_mutex);
    sem_destroy(&sem_postazioni);
    for (int i = 0; i < N; i++) {
        sem_destroy(&sem_chitarre[i]);
    }
    
    printf("Simulazione terminata\n");
    
    return 0;
}