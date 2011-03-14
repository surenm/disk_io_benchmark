#include "main.h"

int main(int argc, char **argv) {
    int c ;

    // Type of Benchmark - read or write, default to read
    string io_type = "read";

    // Number of threads
    // default 1
    int thread_count = 1;

    // If its a write, number of blocks/files to write
    // default 1
    int blocks_count = 1;

    // Size of the block to be read in MB
    // default 64 MB
    int block_size = 64;

    // File/Directory location
    string path;

    //Generic argument parser code
    while(1){
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            { "read", no_argument, 0, 0 },
            { "write", no_argument, 0, 0 },
            { "path", required_argument, 0, 0},
            { "threads", required_argument, 0, 0},
            { "blocks", optional_argument, 0, 0},
            { "block_size", required_argument, 0, 0},
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
						path = string(optarg);
                        break;
                    case 3:
                        thread_count = atoi(optarg);
                        break;
                    case 4:
                        blocks_count = atoi(optarg);
                        break;
                    case 5:
                        block_size = atof(optarg);
                        break;

                    default:
                        break;
                }
                break;
            default :
               	break;
        }
    }

}
