#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

queue *init_queue() {
    queue *kolejka = malloc(sizeof(queue));
    kolejka->next = NULL;
}

void enqueue(queue *first, int id, pthread_cond_t *zmienna) {
    while (first->next != NULL)
           first = first->next;
    first->next = malloc(sizeof(queue));
    first = first->next;
    first->id = id;
    first->zmienna = zmienna;
    first->next = NULL;
}

queue *dequeue(queue *first) {
    queue *second = first->next;
    free(first);
    return second;
}

void show_queue(queue *first) {
    if (first->next == NULL) return;
    first = first->next;
    while (first != NULL) {
        printf("%d ", first->id);
        first = first->next;
    }
}
