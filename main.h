#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <cstring>
#include <ctime>
using namespace std;

#include <unistd.h>
#include <getopt.h>


struct Job {
	int thread_id ;
    int num_blocks;
    int block_size;
    std::string path;
    
    // constructor 
    Job(int id, int blocks, int size, std::string p) :
    	thread_id(id), num_blocks(blocks), block_size(size), path(p) { }
};

