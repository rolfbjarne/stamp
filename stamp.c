#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

static char buffer [1024];
static int index = 0;
static char msg [2048];

struct timeval start_time;
struct timeval previous_time;

static void
get_stamp (struct timeval* time, struct tm* result)
{
	struct timezone tz;

	gettimeofday (time, &tz);
	time->tv_sec += tz.tz_minuteswest * 60;
	localtime_r (&time->tv_sec, result);
}
void
writeline ()
{
	struct timeval time;
	struct tm result;

	get_stamp (&time, &result);

	buffer [index] = 0;

	uint64_t current_us = time.tv_sec * 1000000ULL + time.tv_usec;
	uint64_t previous_us = previous_time.tv_sec * 1000000ULL + previous_time.tv_usec;
	uint64_t diff_previous_us = current_us - previous_us;
	uint64_t start_us = start_time.tv_sec * 1000000ULL + start_time.tv_usec;
	uint64_t diff_start_us = current_us - start_us;

	int len;

	if (previous_us == 0)
		diff_previous_us = 0;
	if (start_us == 0)
		diff_start_us = 0;

	len = snprintf (msg, sizeof (msg), "%.2i:%.2i:%.2i.%.3i (%+3i.%.6i %+3i.%.6i): %s\n", 
		result.tm_hour + 1, result.tm_min, result.tm_sec, time.tv_usec / 1000, 
		(int) (diff_start_us    / 1000000), (int) (diff_start_us    % 1000000),
		(int) (diff_previous_us / 1000000),	(int) (diff_previous_us % 1000000),
		buffer);

	write (1 /* STDOUT_FILENO */, msg, len + 1);
	index = 0;

	previous_time = time;
	if (start_time.tv_sec == 0)
		start_time = time;
}

int main (int argv, char** argc)
{
	tzset ();

	struct timeval time;
	struct tm result;
	get_stamp (&time, &result);

	previous_time = time;
	start_time = time;

	while (1) {
		ssize_t rv = read (0 /* STDIN_FILENO */, buffer + index, 1);
		if (rv == 0) {
			/* eof */
			if (index > 0)
				writeline ();
			return 0;
		} else if (rv < 0) {
			continue;
		}


		if (index >= sizeof (buffer)) {
			writeline ();
		} else if (buffer [index] == '\n' || buffer [index] == '\r') {
			writeline ();
		} else {
			index++;
		}
	}
}