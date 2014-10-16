## For students: change this path to the root of your code
CODEROOT = "../"

#CC = gcc
CC = g++

#CPPFLAGS = -Wall -I$(CODEROOT) $(INCLUDES) -O3  # maximal optimization
CPPFLAGS = -Wall -I$(CODEROOT) $(INCLUDES) -g     # with debugging info

GTEST =  $(CODEROOT)../gtest
LDFLAGS = -L$(GTEST)/lib -L$(CODEROOT)rbf -L$(CODEROOT)rm
GTEST_LIBS = -lgtest_main -lgtest -lpthread
RBFM_LIB = -lrbf
INCLUDES = -I$(GTEST)/include -I$(CODEROOT)rbf -I$(CODEROOT)rm
LDLIBS = $(GTEST_LIBS) $(RBFM_LIB)