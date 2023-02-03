#include "periodic.h"
#define _GNU_SOURCE

// #include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

static void* _PERIODIC_TimerThread(void* arg);

periodic_timer_t* PERIODIC_CreateTimer(void (*cb)(void* arg), void* arg, unsigned int ms) {
	periodic_timer_t* timer = calloc(1, sizeof(periodic_timer_t));
	if (!timer) return 0;
	timer->period_ms = ms;
	timer->cb = cb;
	timer->arg = arg;

	pthread_create(&timer->thread, 0, _PERIODIC_TimerThread, timer);
	return timer;
}

static inline void NoBlocking(int fd) {
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_NONBLOCK);
}

static void* _PERIODIC_TimerThread(void* arg) {
	if (!arg) return 0;
	periodic_timer_t* const timer = arg;

	pipe((int*) timer->pipes);
	NoBlocking(timer->pipes[0]);
	uint8_t buf;
	while (!timer->destroy) {
		struct pollfd pfd = {
			.fd = timer->pipes[0],
			.events = POLLIN | POLLHUP
		};
		poll(&pfd, 1, timer->period_ms);
		if (pfd.revents) {
			if (pfd.revents & POLLIN) {
				read(timer->pipes[0], &buf, 1);
			} else {
				break;
			}
		}
		if (timer->skip > 0) {
			timer->skip--;
		} if (timer->destroy) {
			break; // Better, than skip
		} else {
			if (timer->cb) timer->cb(timer->arg);
		}
	}
	return 0;
}

void PERIODIC_TriggerTimer(periodic_timer_t* timer) {
	if (!timer) return;
	uint8_t buf = 't';
	write(timer->pipes[1], &buf, 1);
}

void PERIODIC_SetSkip(periodic_timer_t* timer, int skip) {
	if (!timer) return;
	timer->skip = skip;
}

void PERIODIC_DestroyTimer(periodic_timer_t* timer) {
	if (!timer) return;
	timer->destroy = true;
	uint8_t buf = 't';
	write(timer->pipes[1], &buf, 1);
	pthread_cancel(timer->thread);
	pthread_join(timer->thread, NULL);
}
