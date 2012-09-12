#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "debug.h"
#include "threadpool.h"
#include "common.h"

static struct timeval begin_time;
static long int late = 0;
static pthread_mutex_t lock;
static long int *temp;
static struct setting setting;
static struct record *records;
static int fd;
static int thread;

static void trace_replay(void *arg)
{
	struct timeval end_time, result_time, test_time;
	/*long int i = *(long int *)arg;*/
	long int i;
	i = *(long int *)arg;
	*(long int *)arg += thread;

	char *buf;
	long long offset;
	long length;

	offset = records[i].offset;
	length = records[i].length;
	buf = (char *)malloc((length + 1) * sizeof(char));
	if (buf == NULL) {
		debug_puts("MEMORY ERROR");
		return ;
	}
/*
	int mod;
*	mod = (long int *)arg - temp; 
*	debug_print("%ld", temp[mod]);
*	temp[mod] = temp[mod] + THREAD;
*/
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &begin_time, &result_time);
	timersub(&records[i].time, &result_time, &test_time);
	do {
		if (test_time.tv_sec < 0) {
			pthread_mutex_lock(&lock);
			late++;
			pthread_mutex_unlock(&lock);
			gettimeofday(&end_time, NULL);
			timersub(&end_time, &begin_time, &result_time);
			records[i].time.tv_sec = result_time.tv_sec;
			records[i].time.tv_usec = result_time.tv_usec;
			break;
		}
		if (test_time.tv_sec != 0) {
			sleep(test_time.tv_sec);
		}
		if (test_time.tv_usec >= 0) {
			usleep(test_time.tv_usec);
			/*
			debug_print("%ld, %ld", i, test_time.tv_usec);
			pthread_mutex_lock(&lock);
			done++;
			pthread_mutex_unlock(&lock);
			*/
		}
	} while(0);
	/*
     * debug_print("%ld", i);
	 */
	/*
	 * TODO:r/w 
	 */
	lseek(fd, offset, SEEK_SET);
	read(fd, buf, length);
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &begin_time, &result_time);
	timersub(&result_time, &records[i].time, &records[i].res_time);
	free(buf);
}

int main(int argc, const char *argv[])
{
	long int i, usec, ret=0;
	threadpool_t *pool;
	struct timeval total_time;
	/*
	 * fraction calucate the fraction of response time appear in <5ms, 10ms, 15ms, 20ms, 25ms, 30ms, >30ms
	 */
	long int fraction[7];
	
	total_time.tv_sec = total_time.tv_usec = 0;

	ret = init_conf(&setting);
	thread = setting.threads_num;

	if(ret) {
		debug_puts("INIT SETTING ERROR");
		return -1;
	}

	temp = (long int *)malloc((thread) * sizeof(long int));
	if (NULL == temp) {
		debug_puts("MEMORY ERROR");
		return -1;
	}
	records = (struct record *)malloc((setting.records_num) * sizeof(struct record));
	if (NULL == records) {
		debug_puts("MEMORY ERROR");
		return -1;
	}
	for (i = 0; i < 7; i++) {
		fraction[i] = 0;
	}
	/*
	 * init temp array for record trace num
	 */
	for (i = 0; i < thread; i++) {
		temp[i] = i;
	}

	/*
	 * read the record file
	 */
	init_rec_array(setting, records);

	/*
	 * init_rec_array test
	 */
	/*
	for (i = 0; i < setting.records_num; i++) {
		printf("%d,%lld,%ld,%c,%ld.%06ld\n", records[i].dev_num, records[i].offset, \
				records[i].length, records[i].op, records[i].time.tv_sec, records[i].time.tv_usec);
	}
	*/

	pthread_mutex_init(&lock, NULL);

	pool = threadpool_init(setting.threads_num, setting.pool_size, 0);

	if(!pool) {
		return -1;
	}

	fd = open(setting.location, O_RDONLY);
	if (fd == -1) {
		debug_puts("DISK ERROR");
		return -1;
	}

	printf("Init over... Start trace play. Begin time is Beijing time:\n");
	time_t p_time = time(NULL);
	printf("%s\n", ctime(&p_time));

	gettimeofday(&begin_time, NULL);

	/*
	 * add trace play to the threadpool and begin the trace play
	 */
	for (i = 0; i < setting.records_num; i++) {
		/*ret = threadpool_add(pool, &trace_replay, (void *)&temp[i], 0);
		 */
		ret = threadpool_add(pool, &trace_replay, (void *)&temp[i % thread], 0);
		if (ret) {
			debug_puts("TASK ADD ERROR");
			break;
		}
	}
	
	sleep(1);
	threadpool_destroy(pool, 0);
	close(fd);

	printf("Trace play over, %ld traces are late. now calculate total response time.\n", late);
	for (i = 0; i < setting.records_num; i++) {
		timeradd(&total_time, &records[i].res_time, &total_time);
		usec = records[i].res_time.tv_usec;
		if ((usec / 5000) > 6) {
			fraction[6]++;
		}else {
			fraction[usec / 5000]++;
		}
	}
	printf("Play %d traces in %ld.%06lds\n", setting.records_num, total_time.tv_sec, total_time.tv_usec);
	printf("<5ms:%ld\t10ms:%ld\t15ms:%ld\t20ms:%ld\t25ms:%ld\t30ms:%ld\t>30ms:%ld\n", \
			fraction[0], fraction[1], fraction[2], fraction[3], fraction[4], \
			fraction[5], fraction[6]);
	return 0;
}
