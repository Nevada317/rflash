#ifndef _PERIODIC_H
#define _PERIODIC_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct periodic_timer_t {
	unsigned int period_ms;
	volatile int pipes[2];
	volatile int skip;
	volatile bool destroy;
	void* arg;
	pthread_t thread;
	void (*cb)(void* arg);
} periodic_timer_t;

periodic_timer_t* PERIODIC_CreateTimer(void (*cb)(void* arg), void* arg, unsigned int ms);
void PERIODIC_TriggerTimer(periodic_timer_t* timer);
void PERIODIC_SetSkip(periodic_timer_t* timer, int skip);
void PERIODIC_DestroyTimer(periodic_timer_t* timer);

#endif /* _PERIODIC_H */
