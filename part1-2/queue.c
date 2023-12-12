#include <stdlib.h>
#include "queue.h"

struct Queue {
	size_t cap, size, head, tail;
	int data[];
};

Queue *Queue_Create(size_t size) {
	Queue *queue = malloc(sizeof(Queue) + sizeof(int) * size);
	*queue = (Queue) {
		.cap = size,
		.size = 0,
		.head = 0,
		.tail = 0
	};
	return queue;
}

void Queue_Destroy(Queue *queue) {
	free(queue);
}

int Queue_Enqueue(Queue *queue, int data) {
	if (Queue_IsFull(queue)) {
		return 0;
	}
	queue->data[queue->tail++] = data;
	queue->tail %= queue->cap;
	++queue->size;
	return 1;
}

int Queue_Dequeue(Queue *queue, int *data) {
	if (Queue_IsEmpty(queue)) {
		return 0;
	}
	if (data) *data = queue->data[queue->head++];
	else ++queue->head;
	queue->head %= queue->cap;
	--queue->size;
	return 1;
}

int Queue_Front(Queue *queue) {
	return queue->data[queue->head];
}

int Queue_Rear(Queue *queue) {
	return queue->data[queue->tail - 1];
}

int Queue_IsFull(Queue *queue) {
	return queue->size == queue->cap;
}

int Queue_IsEmpty(Queue *queue) {
	return queue->size == 0;
}

size_t Queue_Size(Queue *queue) {
	return queue->size;
}
