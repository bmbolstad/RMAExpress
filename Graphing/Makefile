### file: Makefile.unx
### 
### This Makefile is for compiliation on a Linux machine 
### .
###
###
###


WXINCLUDE = `wx-config --cppflags`
WXLIB = `wx-config --libs` 

WXBASEINCLUDE = `wx-config --cppflags`           
WXBASELIB = `wx-config --libs base,core`         

COMPILERFLAGS = -g -O2 ########-D BUFFERED -D RMA_GUI_APP
COMPILERFLAGSBASE = -g -O2 ############-D BUFFERED 

CC = g++

all: GraphicTester


clean:
	rm *.o GraphingTester


GraphicTester: GraphingTester.cpp boxplot.o axes.o
	$(CC) $(COMPILERFLAGS) GraphingTester.cpp  $(WXINCLUDE) boxplot.o axes.o $(WXLIB)  -o GraphingTester

boxplot.o: boxplot.cpp boxplot.h
	$(CC) -c  $(COMPILERFLAGS) boxplot.cpp $(WXINCLUDE)  -o boxplot.o

axes.o: axes.cpp axes.h
	$(CC) -c  $(COMPILERFLAGS) axes.cpp $(WXINCLUDE)  -o axes.o



