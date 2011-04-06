#include <string>
#include <vector>
#include <ctime>
using namespace std;

struct Job {
	int thread_id ;
	vector<string> block_names;
	unsigned long long block_size;
	int chunk_size;
	time_t elapsed_time ;
};

int do_IO( string io_action, string path, int thread_count, int chunk_size,
		int block_size, int blocks_count );

unsigned long long get_file_size(string file_name);

vector<string> get_dir_listing(string path);

//Thread callback functions
void* read(void* data);
void* write(void* data);
void* read_buffered(void* data);
void* write_buffered(void* data);


