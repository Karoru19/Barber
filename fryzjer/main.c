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
int licznik = 0, zrezygnowani = 0, klient = -1, debug = 0;
sem_t budzenie, zajetosc_fryzjera, strzyzenie_klienta, customerLock2, poczekalnia;

queue *oczekujacy_klienci, *zrezygnowani_klienci;

int * parser(int argc, char* argv[])
{
  static int config[3];  //0 - time, 1 - debug (0 -tak, 1 - nie), 2 - rozmiar poczekalni

  config[0] = 10; //domyślny time
  config[1] = 5;  //domyślny rozmiar poczekalni
  config[2] = 0; //domyślnie no debug
  int j;

  if(argc > 2 && argc < 7)
  {
    for (j=0;j<argc;j++)
    {
      if(strcmp(argv[j], "-t") == 0)
      {
        int len = strlen(argv[j+1]), i;
        for(i=0;i<len;i++)
        {
          if (!isdigit(argv[j+1][i]))
              return config;
        }
        config[0] = atoi(argv[j+1]);  //us
        j++;
      }
      else if(strcmp(argv[j], "-ts") == 0)
      {
        int len = strlen(argv[j+1]), i;
        for(i=0;i<len;i++)
        {
          if (!isdigit(argv[j+1][i]))
              return config;
        }
        config[0] = atoi(argv[j+1]) * 1000000;  //s -> us
        j++;
      }
      else if(strcmp(argv[j], "-tms") == 0)
      {
        int len = strlen(argv[j+1]), i;
        for(i=0;i<len;i++)
        {
          if (!isdigit(argv[j+1][i]))
              return config;
        }
        config[0] = atoi(argv[j+1]) * 1000;  //ms -> us
        j++;
      }
      else if(strcmp(argv[j], "-size") == 0 || (strcmp(argv[j], "-s") == 0))
      {
        int len = strlen(argv[j+1]), i;
        for(i=0;i<len;i++)
        {
          if (!isdigit(argv[j+1][i]))
              return config;
        }
        config[1] = atoi(argv[j+1]);  //rozmiar poczekalni
        j++;
      }
      else if(strcmp(argv[j], "-debug") == 0)
      {
        config[2] = 1;
      }
    }
  }

    return config;
}

void state(){
    int value;
    sem_getvalue(&poczekalnia, &value);
    value = 10 - value;
    if (klient < 0)
        printf("Res: %d Wromm: %d/10 [in: Brak]\n", zrezygnowani, value);
    else
        printf("Res: %d Wromm: %d/10 [in: %d]\n", zrezygnowani, value, klient);
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
        sem_post(&poczekalnia);

        sem_wait(&strzyzenie_klienta);
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
    pthread_t* customer_thread = malloc(sizeof(pthread_t));
    pthread_create(customer_thread, NULL, (void*) customer, NULL);
}

void semaphoreInit(){
    sem_init(&budzenie, 0, 0);
    sem_init(&zajetosc_fryzjera, 0, 0);
    sem_init(&strzyzenie_klienta, 0, 0);
    sem_init(&poczekalnia, 0, 10);
}

int main(int argc,char *argv[]) {
    semaphoreInit();
    zrezygnowani_klienci = init_queue();
    oczekujacy_klienci = init_queue();
    setbuf(stdout, NULL);
    debug = 1;
    pthread_create(&thread_fryzjer, NULL, (void*) barber, NULL);
    while(1) {
        createCustomer();
        usleep(75000);
    }
}
