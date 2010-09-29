CC=gcc 
CPPFLAGS= -g -O3 -mtune=native -msse3 `pkg-config gstreamer-0.10 --cflags` 

OUT=libofxGstVideoRecorder.a
OBJ=ofxGstVideoRecorder.o ofGstUtils.o

$(OUT): $(OBJ) $(wildcard *.h) $(wildcard ../FProfiler/*.h)
	ar rcs $(OUT) $(OBJ)

profile: CPPFLAGS+=-DPROFILE
profile: all

clean:
	rm -f $(OUT) $(OBJ)

all: $(OUT)

