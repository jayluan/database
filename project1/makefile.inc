## For students: change this path to the root of your code
CODEROOT = "/home/jay/dev/cs222_data_management/project1"

#CC = gcc
CC = g++

#CPPFLAGS = -Wall -I$(CODEROOT) $(INCLUDES) -O3  # maximal optimization
CPPFLAGS = -Wall -I$(CODEROOT) $(INCLUDES) -g     # with debugging info

GTEST = /home/jay/dev/cs222_data_management/gtest
LFLAGS = -L$(GTEST)/lib 
FULLLIBS = -l$(GTEST)/lib/libgtest -l$(GTEST)/lib/libgtest_main
LIBS = -lgtest_main -lgtest -lpthread
INCLUDES = -I$(GTEST)/include