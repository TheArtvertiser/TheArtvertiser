CC=gcc 
CPPFLAGS=-g -o2

OUT=libfprofiler.a
OBJ=FProfiler.o FTime.o FThread.o 

$(OUT): $(OBJ) $(wildcard *.h)
	ar rcs $(OUT) $(OBJ)

profile: CPPFLAGS+=-DPROFILE
profile: all

clean:
	rm -f $(OUT) $(OBJ)

all: $(OUT)

