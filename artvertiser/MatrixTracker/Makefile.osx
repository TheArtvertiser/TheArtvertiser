CC=gcc 
CPPFLAGS= -g -O3 -mtune=native -msse3 -arch i386 -I/usr/local/include/opencv

OUT=libMatrixTracker.a
OBJ=MatrixTracker.o ofxMatrix3x3.o ofxMatrix4x4.o ofxQuaternion.o

$(OUT): $(OBJ) $(wildcard *.h) $(wildcard ../FProfiler/*.h)
	ar rcs $(OUT) $(OBJ)

profile: CPPFLAGS+=-DPROFILE
profile: all

clean:
	rm -f $(OUT) $(OBJ)

all: $(OUT)

