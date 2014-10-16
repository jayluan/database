
include ../makefile.inc

all: rbf_unittest

rbf_unittest: rbf_unittest.o $(CODEROOT)/rbf/librbf.a

rbf_unittest.o: rbf_unittest.h

# dependencies to compile used libraries
.PHONY: $(CODEROOT)/rbf/librbf.a
$(CODEROOT)/rbf/librbf.a:
	$(MAKE) -C $(CODEROOT)/rbf librbf.a

.PHONY: clean
clean:
	-rm rbf_unittest *.a *.o *~
	$(MAKE) -C $(CODEROOT)/rbf clean
