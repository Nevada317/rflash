#include "sleep.h"
#include <time.h>
#include <errno.h>

// int nanosleep(const struct timespec *req, struct timespec *rem);

int sleep_ms(long ms, bool interruptible) {
	int res = 0;
	struct timespec ts = {0,};

	if (ms <= 0) {
		return EINVAL;
	}
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	do {
		errno = 0;
		res = nanosleep(&ts, &ts);
		if (interruptible && (errno == EINTR)) break;
	} while (res && errno == EINTR);

		return res;
}

