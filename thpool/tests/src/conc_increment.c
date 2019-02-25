#include "iot/thpool.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int sum=0;


void increment() {
	pthread_mutex_lock(&mutex);
	sum ++;
	pthread_mutex_unlock(&mutex);
}


int main(int argc, char *argv[]){
	
	char* p;
	if (argc != 3){
		puts("This testfile needs excactly two arguments");
		exit(1);
	}
	int num_jobs    = strtol(argv[1], &p, 10);
	int num_threads = strtol(argv[2], &p, 10);

	threadpool thpool = iot_thpool_init(num_threads);
	
	int n;
	for (n=0; n<num_jobs; n++){
		iot_thpool_add_work(thpool, (void*)increment, NULL);
	}
	
	iot_thpool_wait(thpool);

	printf("%d\n", sum);

	return 0;
}
