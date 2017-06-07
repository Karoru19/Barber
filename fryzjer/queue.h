#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

typedef struct queue queue;

struct queue {
    int id;
    pthread_cond_t *zmienna;
    queue *next;
};

queue* init_queue();
void enqueue(queue *first, int id, pthread_cond_t *zmienna);
queue* dequeue(queue *first);
void show_queue(queue *first);

#endif
