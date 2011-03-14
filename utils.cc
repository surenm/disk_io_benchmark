#include <pthread.h>
#include <set>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

void *HelloWorld(void *t){
	int temp;
	temp = (long)t;
	sleep(1);
	pthread_exit(NULL);
}

int do_IO( string io_action, string path, int thread_count, int block_size, int blocks_count ){
	vector<pthread_t> threads(thread_count);
	int return_code ;

	int start = time(NULL);

	for(int i=0; i<thread_count; i++){
		return_code = pthread_create(&threads[i], NULL, HelloWorld, (void *)i);
		if(return_code) {
			cout << "Error in creating thread " << i << endl;
			exit(-1);
		}
	}

	for(int i=0; i<thread_count; i++){
		void *status ;
		return_code = pthread_join(threads[i], &status);
		if(return_code) {
			cout << "Could not join thread " << i << endl;
			exit(-1);
		}
	}

	int end = time(NULL);

	cout << end - start << endl ;

	return 0;
}
