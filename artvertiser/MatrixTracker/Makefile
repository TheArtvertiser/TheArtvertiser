CC=gcc 
CPPFLAGS=-g -o2 -I/usr/local/include/opencv

OUT=libMatrixTracker.a
OBJ=MatrixTracker.o ofxMatrix3x3.o ofxMatrix4x4.o ofxQuaternion.o 

%.o: %.cpp $(wildcard *.h)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

profile: CPPFLAGS+=-DPROFILE
profile: all

clean:
	rm -f $(OUT) $(OBJ)

all: $(OUT)

