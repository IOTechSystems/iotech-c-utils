#include "iot/thpool.h"


int main(int argc, char *argv[]){

	char* p;
	if (argc != 2){
		puts("This testfile needs exactly one arguments");
		exit(1);
	}
	int num_threads = strtol(argv[1], &p, 10);

	threadpool thpool = iot_thpool_init(num_threads);
	iot_thpool_destroy(thpool);

	sleep(1); // Sometimes main exits before iot_thpool_destroy finished 100%

	return 0;
}
