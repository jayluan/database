## For students: change this path to the root of your code
CODEROOT = "/home/jay/dev/cs222_data_management/project1"

#CC = gcc
CC = g++

#CPPFLAGS = -Wall -I$(CODEROOT) $(INCLUDES) -O3  # maximal optimization
CPPFLAGS = -Wall -I$(CODEROOT) $(INCLUDES) -g     # with debugging info

LFLAGS = -L../../gtest/lib
LIBS = -llibgtest
INCLUDES = -I../../gtest/include