#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
using namespace std;

#include <unistd.h>
#include <getopt.h>

#include "utils.h"

void print_usage(void){
	cout << "Usage : \n"
			"	disk_io_benchamarks [options] path[s]\n"
			"Options: \n"
			" --read        : if the IO operation to be performed is read.\n"
			" --write       : if the IO operation to be performed is write.\n"
			" --threads     : Number of threads of execution.\n"
			" --help        : Print this help.\n"
			" --blocks      : Number of blocks/files to be written.\n"
			" --block_size  : Size of each block to be written.\n"
			" --chunk_size  : Size of each small chunk to be read/written\n"
			" --buffered    : Does a buffered version of IO\n"
			<< endl;
	return ;
}

int main(int argc, char **argv) {

	if(argc < 2){
    	print_usage();
    	return 0;
    }

    // Type of Benchmark - read or write, default to read
    string io_type = "read";

    // File/Directory location
    vector<string> path;

    // Number of threads
    // default 1
    int thread_count = 1;

    // If its a write, number of blocks/files to write
    // default 1
    int blocks_count = 1;

    // Size of the each chunk to be read/written
    // default 4 KB
    int chunk_size = 4096;

    // default block size to be read/written
    // 100 MB
    unsigned long long block_size = 100 * 1024 *1024 ;

    int c ;

    bool buffered = false ;

    //Generic argument parser code
    while(1){
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            { "read", no_argument, 0, 0 },
            { "write", no_argument, 0, 0 },
            { "threads", required_argument, 0, 0},
            { "blocks", required_argument, 0, 0},
            { "block_size", required_argument, 0, 0},
            { "chunk_size", required_argument, 0, 0},
            { "buffered", no_argument, 0, 0},
            { "help", no_argument, 0 ,0},
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if(c == -1)
            break;

        switch(c){
            case 0:
#ifdef DEBUG
            	cout << "Option - " << long_options[option_index].name ;
                if(optarg)
                    cout << " : " << optarg ;
                cout << endl ;
#endif
                switch(option_index){
                    case 0:
                        io_type = string(long_options[option_index].name);
                        break;
                    case 1:
                    	io_type = string(long_options[option_index].name);
                        break;
                    case 2:
                        thread_count = atoi(optarg);
                        break;
                    case 3:
                        blocks_count = atoi(optarg);
                        break;
                    case 4:
                        block_size = atol(optarg);
                        break;
                    case 5:
                    	chunk_size = atoi(optarg);
                    	break;
                    case 6:
                    	buffered = true;
                    case 7:
                    	print_usage();
                    	break;
                    default:
                        break;
                }
            break;
            case '?' :
            	print_usage();
            	break;
            default :
               	abort;
        }

    }

    for(int index = optind; index < argc; index++){
        
        string _path = string(argv[index]);
        if(_path[_path.size()-1] != '/') _path += '/' ;
        path.push_back(_path);
        
    }
#ifdef DEBUG 
    cout << "Path(s): " << endl ;
    for(int i=0; i<path.size(); i++)
        cout  << path[i] << endl;
#endif
    int rc = do_IO(io_type, path, thread_count, chunk_size, block_size, blocks_count);

    return 0;

}
