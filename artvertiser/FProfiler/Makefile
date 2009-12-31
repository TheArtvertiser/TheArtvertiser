CC=gcc 
CFLAGS=-g -o2

OUT=libfprofiler.a
OBJ=FProfiler.o FTime.o FThread.o

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f libfprofiler.a FProfiler.o FTime.o

all: $(OUT)

