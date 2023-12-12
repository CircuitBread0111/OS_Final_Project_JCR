#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue Queue;

Queue *Queue_Create(size_t size);

void Queue_Destroy(Queue *queue);

int Queue_Enqueue(Queue *queue, int data);

int Queue_Dequeue(Queue *queue, int *data);

int Queue_Front(Queue *queue);

int Queue_Rear(Queue *queue);

int Queue_IsFull(Queue *queue);

int Queue_IsEmpty(Queue *queue);

size_t Queue_Size(Queue *queue);

#endif
