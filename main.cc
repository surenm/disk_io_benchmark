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
#define DEBUG

void print_usage(void){
	cout << "Usage : \n"
			"	disk_io_benchamarks [options] path\n"
			"Options: \n"
			" --read        : if the IO operation to be performed is read.\n"
			" --write       : if the IO operation to be performed is write.\n"
			" --threads     : Number of threads of execution.\n"
			" --help        : Print this help.\n"
			" --blocks      : Number of blocks/files to be written.\n"
			" --block_size  : Size of each block to be written.\n"
			" --chunk_size  : Size of each small chunk to be read/written"
			<< endl;
	return ;
}

int main(int argc, char **argv) {
    int c ;

    if(argc < 2){
    	print_usage();
    	return 0;
    }

    // Type of Benchmark - read or write, default to read
    string io_type = "read";

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
    int block_size = 100 * 1024 *1024 ;

    // File/Directory location
    string path;

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
                        block_size = atoi(optarg);
                        break;
                    case 5:
                    	chunk_size = atoi(optarg);
                    	break;
                    case 6:
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
    path = string(argv[optind]);
    cout << "Path: " << path << endl;
    int rc = do_IO(io_type, path, thread_count, chunk_size, block_size, blocks_count);

    return 0;

}
