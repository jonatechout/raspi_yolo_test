CC = g++
CFLAGS = -g -std=c++11 -O3 -Wall -DOPENCV -DNNPACK -DARM_NEON -mfpu=neon-vfpv4 -funsafe-math-optimizations -ftree-vectorize 
SRCS = main.cpp
INCS = -I../darknet-nnpack/include/
PROG = raspiyolotest

OPENCV = `pkg-config --cflags --libs opencv`
LIBS = $(OPENCV) -L../darknet-nnpack/  -lnnpack -lpthreadpool -lpthread -ldarknet -lraspicam_cv

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(INCS) $(LIBS) 
