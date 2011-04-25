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
	int length = job->chunk_size ;

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

	int length = job->chunk_size;

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
	pthread_exit(0);
}

void* write(void* data){
	Job *job = (Job *) data;

	int chunk_size = job->chunk_size ;

	char *s = (char *) malloc(chunk_size * sizeof(char));

	for (int i = 0; i < job->block_names.size(); i++) {
		unsigned long long chunks = job->block_size / chunk_size + 1;
		FILE* fptr = fopen(job->block_names[i].c_str(),"w+");
		fclose(fptr);
		size_t fd = open(job->block_names[i].c_str(), O_WRONLY | O_APPEND);
		memset(s, '0', chunk_size*sizeof(char));

		// Measure time for reading the file in entirety
		time_t start_time = time(NULL);
		while( write(fd, s, chunk_size) > 0 && chunks--);
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
		cerr << "Error " << errno << " opening " << path << endl;
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

inline unsigned long long get_file_size(string file_name){
    //stat the file and get size
    struct stat st ;
    int return_code = stat(file_name.c_str(), &st);
    return st.st_size;
}

int do_IO( string io_action, vector<string> paths, int thread_count, int chunk_size,
    	unsigned long long block_size, int blocks_count ){

	// Thread Identifiers for each thread that is going to be spawn
	vector<pthread_t> threads(thread_count);

	// List of all files to be totally read/written
	vector<string> blocks_uri;

    // Throughput for each thread 
    vector<double> throughput;

	// Return code for various exit points in the code
	int return_code ;

	// Declare the Thread callback function pointer
	void* (*thread_fp)(void*);

	//Handle different types of IO actions
	if(io_action == "read") {
	
        // set the thread callback to read
		thread_fp = &read;

        for( int i=0; i<paths.size(); i++){

            string path = paths[i];
            // Check if the @path given is a file or directory listing
            // Get the directory listing; if its a file just add that file alone
            // as the file listing
            struct stat _stat ;
            return_code = stat(path.c_str(), &_stat);
            if(return_code){
                cerr << "Could not open path " << path << endl;
                exit(return_code);
            }

            if(S_ISDIR(_stat.st_mode )){
                vector<string> append_vector = get_dir_listing(path);
                blocks_uri.insert(blocks_uri.end(), append_vector.begin(), append_vector.end());
            }
            else
                blocks_uri.push_back(path);
        }
	}

	else if (io_action == "write") {
		// set the thread callback to write
		thread_fp = &write;

        for( int k=0; k<paths.size(); k++){
                string path = paths[k];

                struct stat _stat;
                return_code = stat(path.c_str(), &_stat);
                if (return_code) {
                    cerr << "Could not open path " << path << endl;
                    exit(return_code);
                }
                if	(S_ISDIR(_stat.st_mode )){

                for(int i=0; i<blocks_count; i++){
                    ostringstream ostr ; ostr << i << ".txt";
                    blocks_uri.push_back(path+ostr.str());
                }
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
		jobs[i].chunk_size = chunk_size ;

		end = start + min_blocks_per_thread ;
		if(i < blocks_uri.size() % thread_count) end = end + 1 ;
		jobs[i].block_names = vector<string>(start, end);
		start = end	;

		jobs[i].elapsed_time = 0;

		struct Job *data = &jobs[i];

		return_code = pthread_create(&threads[i], NULL, thread_fp, (void *)(data));
		if(return_code) {
			cerr << "Error in creating thread " << i << endl;
			exit(-1);
		}
	}

	time_t start_time = time(NULL);
	// Wait for all the threads to complete
	for(int i=0; i<thread_count; i++){
		void *status ;
		return_code = pthread_join(threads[i], &status);
		if(return_code) {
			cerr << "Could not join thread " << i << endl;
			exit(-1);
		}
	}
	// The IO Operation ends here

	time_t end_time = time(NULL);

    // Do some non sense and print the output in csv format

    int total_time_taken = end_time - start_time ; 
    double total_io = 0 ;
    for(int i=0; i<blocks_uri.size(); i++){
        total_io += double(get_file_size(blocks_uri[i]))/(1024*1024) ;
    }
    double overall_throughput = total_io/total_time_taken;

    for(int i=0; i<thread_count; i++){
        double total_io_done_this_thread = 0;
        for(int j=0; j < jobs[i].block_names.size();  j++){
            total_io_done_this_thread += double(get_file_size(jobs[i].block_names[j]))/(1024*1024);
        }
        throughput.push_back(total_io_done_this_thread/jobs[i].elapsed_time);
    }

    cout << "I/O operation ,Chunk size(in Bytes), Total I/O done (in MB), Total time taken"
            "(in seconds), Threads used, Number of disks, Overall Throughput" 
            " (in MB/s ), " ;

    for(int i=0; i<thread_count; i++){
        cout <<"Thread #" << i+1 << "(in MB/s)" ;
        if( i < paths.size()-1 ) cout << ", " ;
    }
    cout << endl ;

    cout << io_action << ", " 
         << chunk_size << ", "
         << total_io << ", " 
         << total_time_taken << ", "
         << thread_count << ", "
         << paths.size() << ", "
         << overall_throughput << ", ";
    
    for( int i=0; i<thread_count; i++){
        cout << throughput[i] ;
        if( i < thread_count - 1 ) cout <<", ";
    }

    cout << endl;

    return 0;
}


