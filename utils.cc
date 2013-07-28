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
#include <cstring>
using namespace std;

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "utils.h"

void* read_buffered(void* data){
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
	free(s);
	pthread_exit(0);
}

void* write_buffered(void* data){
	pthread_exit(NULL);
}

void* read(void* data) {
	Job *job = (Job *) data;

	int length = job->block_size;

	char *s = (char *) malloc(length * sizeof(char));
	for (int i = 0; i < job->block_names.size(); i++) {
		size_t fd = open(job->block_names[i].c_str(), O_RDONLY);

		// Measure time for reading the file in entirety
		time_t start_time = time(NULL);
		while( read(fd, s, length) > 0 );
		time_t end_time = time(NULL);

		job->elapsed_time += end_time - start_time;

		close(fd);
	}
	free(s);
	cout << "Time of execution of thread " << job->thread_id << " is "
			<< job->elapsed_time << endl ;
	pthread_exit(0);
}

void* write(void* data){
	Job *job = (Job *) data;

	char *s = (char *) malloc(1024 * sizeof(char));
	for (int i = 0; i < job->block_names.size(); i++) {
		int length = job->block_size/1024;
		FILE* fptr = fopen(job->block_names[i].c_str(),"w+");
		fclose(fptr);
		size_t fd = open(job->block_names[i].c_str(), O_WRONLY | O_APPEND);
		memset(s, '1', length*sizeof(char));
		// Measure time for reading the file in entirety
		time_t start_time = time(NULL);
		while( write(fd, s, 1024) > 0 && length--);
		time_t end_time = time(NULL);

		job->elapsed_time += end_time - start_time;

		close(fd);

	}

	free(s);
	pthread_exit(0);
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

	// List of all files to be totally read/written
	vector<string> blocks_uri;

	// Return code for various exit points in the code
	int return_code ;

	// Declare the Thread callback function pointer
	void* (*thread_fp)(void*);

	//Handle different types of IO actions
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
		struct stat _stat;
		return_code = stat(path.c_str(), &_stat);
		if (return_code) {
			cout << "Could not open path " << path << endl;
			exit(return_code);
		}

		if	(S_ISDIR(_stat.st_mode )){
			for(int i=0; i<blocks_count; i++){
				ostringstream ostr ; ostr << i << ".txt";
				blocks_uri.push_back(path+ostr.str());
			}
		}

	}

	// If number of threads higher than the files to be read/write, reduce the
	// thread count
	if(blocks_uri.size() < thread_count) thread_count = blocks_uri.size();

	// calculate the blocks/files to be allocated per thread
	int min_blocks_per_thread = blocks_uri.size() / thread_count;

	// The IO operation starts here

	// Spawn all the IO threads that we need
	vector<struct Job> jobs(thread_count);
	vector<string>::iterator start, end ;
	start = blocks_uri.begin() ;

	for(int i=0; i<thread_count; i++){
		jobs[i].thread_id = i;
		jobs[i].block_size = block_size ;

		end = start + min_blocks_per_thread ;
		if(i < blocks_uri.size() % thread_count) end = end + 1 ;
		jobs[i].block_names = vector<string>(start, end);
		start = end	;

		jobs[i].elapsed_time = 0;

		struct Job *data = &jobs[i];

		return_code = pthread_create(&threads[i], NULL, thread_fp, (void *)(data));
		if(return_code) {
			cout << "Error in creating thread " << i << endl;
			exit(-1);
		}
	}

	time_t start_time = time(NULL);
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

	time_t end_time = time(NULL);

	cout << "Total time of threads execution "<< end_time - start_time << endl;

	return 0;
}
