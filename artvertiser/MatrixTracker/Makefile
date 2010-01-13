CC=gcc 
CPPFLAGS=-g -o2

OUT=libMatrixTracker.a
OBJ=MatrixTracker.cpp ofxMatrix3x3.o ofxMatrix4x4.o ofxQuaternion.o

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

profile: CPPFLAGS+=-DPROFILE
profile: all

clean:
	rm -f $(OUT) $(OBJ)

all: $(OUT)

