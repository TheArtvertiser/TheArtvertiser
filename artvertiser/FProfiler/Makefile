CC=gcc 
CPPFLAGS=-g -o2

OUT=libfprofiler.a
OBJ=FProfiler.o FTime.o FThread.o

%.o: %.cpp $(wildcard *.h)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

profile: CPPFLAGS+=-DPROFILE
profile: all

clean:
	rm -f $(OUT) $(OBJ)

all: $(OUT)

