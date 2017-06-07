#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>
#include "queue.h"

pthread_mutex_t mutex_poczekalnia = PTHREAD_MUTEX_INITIALIZER, mutex_gabinet = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_fryzjer;
int licznik = 0, zrezygnowani = 0, klient = -1;
sem_t budzenie, zajetosc_fryzjera, strzyzenie_klienta, poczekalnia;

queue *oczekujacy_klienci, *zrezygnowani_klienci;

int czas, rozmiar, debug;

void parser(int argc, char* argv[]) {
    czas = 75000; //domyślny time 75 ms
    rozmiar = 10;  //domyślny rozmiar poczekalni
    debug = 0; //domyślnie no debug

    if(argc >= 2 && argc < 7) {
        for (int j=0; j < argc; j++) {
            if(strcmp(argv[j], "-t") == 0) {
                int len = strlen(argv[j+1]);
                for(int i = 0; i < len; i++) if (!isdigit(argv[j+1][i])) return;
                czas = atoi(argv[j+1]);  //us
                j++;
            }
            else if(strcmp(argv[j], "-ts") == 0) {
                int len = strlen(argv[j+1]);
                for(int i=0;i<len;i++) if (!isdigit(argv[j+1][i])) return;
                czas = atoi(argv[j+1]) * 1000000;  //s -> us
                j++;
            }
            else if(strcmp(argv[j], "-tms") == 0) {
                int len = strlen(argv[j+1]);
                for(int i = 0; i < len; i++) if (!isdigit(argv[j+1][i])) return;
                czas = atoi(argv[j+1]) * 1000;  //ms -> us
                j++;
            }
            else if(strcmp(argv[j], "-size") == 0 || (strcmp(argv[j], "-s") == 0)) {
                int len = strlen(argv[j+1]);
                for(int i = 0; i < len; i++) if (!isdigit(argv[j+1][i])) return;
                rozmiar = atoi(argv[j+1]);  //rozmiar poczekalni
                j++;
            }
            else if(strcmp(argv[j], "-debug") == 0) debug = 1;
    }
  }
}

void state(){
    int value;
    sem_getvalue(&poczekalnia, &value);
    printf("%d\n", value);
    value = rozmiar - value;
    if (klient < 0)
        printf("Res: %d Wromm: %d/%d [in: Brak]\n", zrezygnowani, value, rozmiar);
    else
        printf("Res: %d Wromm: %d/%d [in: %d]\n", zrezygnowani, value, rozmiar, klient);
    if (debug == 1) {
        printf("waiting clients: < ");
        show_queue(oczekujacy_klienci);
        printf(">\nresigned clients: < ");
        show_queue(zrezygnowani_klienci);
        printf(">\n");
    }
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
    pthread_mutex_lock(&mutex_poczekalnia);
    int id = licznik++;
    if(sem_trywait(&poczekalnia) == 0){
        enqueue(oczekujacy_klienci, id, NULL);
        state();
        //printf("Siadam na krzeszło: %d\n", id);
        sem_post(&budzenie);
        pthread_mutex_unlock(&mutex_poczekalnia);

        pthread_mutex_lock(&mutex_gabinet);

        sem_wait(&strzyzenie_klienta);
        sem_post(&poczekalnia);
        //printf("Strzyżenie klienta o numerze: %d\n", id);
        klient = id;
        oczekujacy_klienci = dequeue(oczekujacy_klienci);
        state();
        usleep(100000);
        sem_post(&zajetosc_fryzjera);
        klient = -1;
        state();
        pthread_mutex_unlock(&mutex_gabinet);
    }
    else {
        ++zrezygnowani;
        enqueue(zrezygnowani_klienci, id, NULL);
        state();
        pthread_mutex_unlock(&mutex_poczekalnia);
        //printf("Zrezygnowałem\n");
    }
}

void createCustomer() {
    pthread_t* thread_klient = malloc(sizeof(pthread_t));
    pthread_create(thread_klient, NULL, (void*) customer, NULL);
}

void semaphoreInit(){
    sem_init(&budzenie, 0, 0);
    sem_init(&zajetosc_fryzjera, 0, 0);
    sem_init(&strzyzenie_klienta, 0, 0);
    sem_init(&poczekalnia, 0, rozmiar);
}

int main(int argc,char *argv[]) {
    parser(argc, argv);
    semaphoreInit();
    zrezygnowani_klienci = init_queue();
    oczekujacy_klienci = init_queue();
    setbuf(stdout, NULL);
    pthread_create(&thread_fryzjer, NULL, (void*) barber, NULL);
    while(1) {
        createCustomer();
        usleep(czas);
    }
}
