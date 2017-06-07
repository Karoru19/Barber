#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t mutex_poczekalnia = PTHREAD_MUTEX_INITIALIZER, mutex_gabinet = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_fryzjer;
int licznik = 0, zrezygnowani = 0, klient = -1, debug = 0;
sem_t budzenie, zajetosc_fryzjera, strzyzenie_klienta, customerLock2, poczekalnia;

void state(){
    int value;
    sem_getvalue(&poczekalnia, &value);
    value = 10 - value;
    if (klient < 0)
        printf("Res: %d Wromm: %d/10 [in: Brak]\n", zrezygnowani, value);
    else
        printf("Res: %d Wromm: %d/10 [in: %d]\n", zrezygnowani, value, klient);
    if (debug == 1)
        printf("Res: %d Wromm: %d/10 [in: %d]\n", zrezygnowani, value, klient);
}

void barber(void* arg) {
    while(1) {
        sem_wait(&budzenie);
        pthread_mutex_lock(&mutex_poczekalnia);
        //printf("Rozpoczęcie strzyżenia\n");
        sem_post(&strzyzenie_klienta);
        pthread_mutex_unlock(&mutex_poczekalnia);
        sem_wait(&zajetosc_fryzjera);
        //printf("Zakończenie strzyżenia\n");
        usleep(100000);
        //printf("Czekam\n");
    }
}

void customer(void* arg) {
    //printf("Wbijam do poczekalni\n");
    if(sem_trywait(&poczekalnia) == 0){
        pthread_mutex_lock(&mutex_poczekalnia);
        int id = licznik++;
        state();
        //printf("Siadam na krzeszło: %d\n", id);
        sem_post(&budzenie);
        pthread_mutex_unlock(&mutex_poczekalnia);
        pthread_mutex_lock(&mutex_gabinet);
        sem_post(&poczekalnia);

        sem_wait(&strzyzenie_klienta);
        //printf("Strzyżenie klienta o numerze: %d\n", id);
        klient = id;
        state();
        usleep(100000);
        sem_post(&zajetosc_fryzjera);
        klient = -1;
        state();
        pthread_mutex_unlock(&mutex_gabinet);
    }
    else {
        ++zrezygnowani;
        state();
        //printf("Zrezygnowałem\n");
    }
}

void createCustomer() {
    pthread_t* customer_thread = malloc(sizeof(pthread_t));
    pthread_create(customer_thread, NULL, (void*) customer, NULL);
}

void semaphoreInit(){
    sem_init(&budzenie, 0, 0);
    sem_init(&zajetosc_fryzjera, 0, 0);
    sem_init(&strzyzenie_klienta, 0, 0);
    sem_init(&customerLock2, 0, 0);
    sem_init(&poczekalnia, 0, 10);
}

int main(int argc,char *argv[]) {
    semaphoreInit();
    setbuf(stdout, NULL);
    pthread_create(&thread_fryzjer, NULL, (void*) barber, NULL);
    while(1) {
        createCustomer();
        usleep(75000);
    }
}
