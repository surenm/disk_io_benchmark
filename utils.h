#include <string>
using namespace std;

struct Job {
	int thread_id ;
    int num_blocks;
    int block_size;
    std::string path;

    // constructor
    Job(int id, int blocks, int size, std::string p) :
    	thread_id(id), num_blocks(blocks), block_size(size), path(p) { }
};

int do_IO( string io_action, string path, int thread_count, int block_size, int blocks_count );

