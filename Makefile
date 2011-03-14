CXXFLAGS =	-O2 -g  -fmessage-length=0
TARGET = disk_io_benchmark
OBJS = *.o *.gch 
all : $(TARGET)

disk_io_benchmark:	main.o utils.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.o utils.o -lpthread
	
main.o : main.cc 
	$(CXX) $(CXXFLAGS) -c main.cc 

utils.o: utils.cc
	$(CXX) $(CXXFLAGS) -c utils.cc utils.h 

clean:
	rm -rf $(TARGET) $(OBJS)  
