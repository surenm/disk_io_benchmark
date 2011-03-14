CXXFLAGS =	-O2 -g -Wall -fmessage-length=0
TARGET = disk_io_benchmark
OBJS = main.o 
all : $(TARGET)

disk_io_benchmark:	main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET)  main.o
	
main.o : main.cc 
	$(CXX) $(CXXFLAGS)  -c main.cc 	main.h

clean:
	rm -rf $(TARGET) $(OBJS)  
