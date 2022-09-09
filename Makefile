CXX = g++ -g -Wall -Werror
CFLAGS = -g -Wall -Werror

all: queue.o label 

clean:
	rm -f *.o  label label_checks autograder_output.bmp autograder_binary.bmp autograder_label2rgb.bmp

%.o: %.cpp %.h
	$(CXX) $(CFLAGS) -c $*.cpp -o $*.o

label: label.cpp queue.o bmplib.o
	$(CXX) $@.cpp queue.o bmplib.o -o $@


