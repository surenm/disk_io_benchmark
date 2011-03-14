#include <string>
#include <vector>
using namespace std;

struct Job {
	int thread_id ;
	vector<string> blocks;
	int block_size;
};

int do_IO( string io_action, string path, int thread_count,
		int block_size, int blocks_count );

vector<string> get_dir_listing(string path);

