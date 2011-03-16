#include <pthread.h>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ios>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cerrno>
using namespace std;

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "utils.h"

void* read(void* data){
	Job *job = (Job *) data;

	ifstream input;

	// conversion from MB to bytes
	int length = job->block_size ;

	char *s = (char *) malloc(length * sizeof(char));
	for(int i = 0; i < job->block_names.size(); i++){
		input.open(job->block_names[i].c_str());
		while(input.good()){
			input.read( s, length);
		}
		input.close();
	}
	pthread_exit(0);
}

void* write(void* data){
	pthread_exit(NULL);
}

vector<string> get_dir_listing(string path){
	// path is already a directory, no need to check
	DIR* dir = opendir(path.c_str());
	if(dir == NULL) {
		cout << "Error " << errno << " opening " << path << endl;
		exit(errno);
	}

	struct dirent *dir_entity;
	vector<string> files;
	while( (dir_entity = readdir(dir)) != NULL){

		if(path[path.size()-1] == '/') path = path.substr(0,path.size()-1);
		string file_name = path + "/" + dir_entity->d_name ;

		struct stat _stat ;
		if(!stat(file_name.c_str(),&_stat)){
			if(!S_ISDIR(_stat.st_mode))
				files.push_back(file_name);
		}
	}

	return files;
}

int do_IO( string io_action, string path, int thread_count, int block_size, int blocks_count ){

	// Thread Identifiers for each thread that is going to be spawn
	vector<pthread_t> threads(thread_count);

	int return_code ;

	// List of all files to be totally read/written
	vector<string> blocks_uri;

	// Declare the Thread callback function pointer
	void* (*thread_fp)(void*);

	if(io_action == "read") {
		// set the thread callback to read
		thread_fp = &read;

		// Check if the @path given is a file or directory listing
		// Get the directory listing; if its a file just add that file alone
		// as the file listing
		struct stat _stat ;
		return_code = stat(path.c_str(), &_stat);
		if(return_code){
			cout << "Could not open path " << path << endl;
			exit(return_code);
		}

		if(S_ISDIR(_stat.st_mode ))
			blocks_uri = get_dir_listing(path);
		else
			blocks_uri.push_back(path);
	}

	else if (io_action == "write") {
		// set the thread callback to write
		thread_fp = &write;
	}

	// The IO operation starts here
	if(blocks_uri.size() < thread_count) thread_count = blocks_uri.size();

	int blocks_per_thread = blocks_uri.size() / thread_count ;



	// Spawn all the IO threads that we need
	vector<struct Job> jobs(thread_count);

	for(int i=0; i<thread_count; i++){

		jobs[i].thread_id = i;
		jobs[i].block_size = block_size ;
		jobs[i].block_names = vector<string>(blocks_uri.begin()+i*blocks_per_thread,
				blocks_uri.begin() + (i+1)*blocks_per_thread);

		struct Job *data = &jobs[i];

		return_code = pthread_create(&threads[i], NULL, thread_fp, (void *)(data));
		if(return_code) {
			cout << "Error in creating thread " << i << endl;
			exit(-1);
		}
	}

	// Wait for all the threads to complete
	for(int i=0; i<thread_count; i++){
		void *status ;
		return_code = pthread_join(threads[i], &status);
		if(return_code) {
			cout << "Could not join thread " << i << endl;
			exit(-1);
		}
	}
	// The IO Operation ends here
	return 0;
}
