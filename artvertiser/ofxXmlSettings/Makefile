CC=gcc 
CPPFLAGS= -g -O3 -mtune=native -msse3 -arch i386

OUT=libofxXmlSettings.a
OBJ=ofxXmlSettings.o tinyxml.o tinyxmlparser.o tinyxmlerror.o

$(OUT): $(OBJ) $(wildcard *.h) $(wildcard ../FProfiler/*.h)
	ar rcs $(OUT) $(OBJ)

profile: CPPFLAGS+=-DPROFILE
profile: all

clean:
	rm -f $(OUT) $(OBJ)

all: $(OUT)

