# openFrameworks universal makefile
#
# make help : shows this message
# make Debug:  makes the application with debug symbols
# make Release: makes the app with optimizations
# make: the same as make Release
# make CleanDebug: cleans the Debug target
# make CleanRelease: cleans the Release target
# make clean: cleans everything
# 
#
# this should work with any OF app, just copy any example
# change the name of the folder and it should compile
# only .cpp support, don't use .c files
# it will look for files in any folder inside the application
# folder except that in the EXCLUDE_FROM_SOURCE variable
# but you'll need to add the include paths in USER_CFLAGS
# add the include paths in the USER_CFLAGS variable
# using the gcc syntax: -Ipath
#
# to add addons to your application, edit the addons.make file
# in this directory and add the names of the addons you want to
# include
#
# edit the following  vars to customize the makefile

EXCLUDE_FROM_SOURCE="bin,.xcodeproj,obj"
USER_CFLAGS = -Istarter -Igarfeild -Iartvertiser -Isrc/ofxThread -Isrc/ofxControlPanel -Iartvertiser/FProfiler -Iartvertiser/MatrixTracker
USER_LD_FLAGS = 
USER_LIBS = 




# you shouldn't modify anything below this line


SHELL =  /bin/sh
CXX =  g++

ARCH = $(shell uname -m)
ifeq ($(ARCH),x86_64)
	LIBSPATH=linux64
	COMPILER_OPTIMIZATION = -march=native -mtune=native -finline-functions -funroll-all-loops  -O3
else ifeq ($(ARCH),armv7l)
	LIBSPATH=linuxarmv7l
	COMPILER_OPTIMIZATION = -march=armv7-a -mtune=cortex-a8 -finline-functions -funroll-all-loops  -O3 -funsafe-math-optimizations -mfpu=neon -ftree-vectorize -mfloat-abi=softfp
else
	LIBSPATH=linux
	COMPILER_OPTIMIZATION = -march=native -mtune=native -finline-functions -funroll-all-loops  -O3
endif

NODEPS = clean CleanDebug CleanRelease
SED_EXCLUDE_FROM_SRC = $(shell echo  $(EXCLUDE_FROM_SOURCE) | sed s/\,/\\\\\|/g)
SOURCE_DIRS = $(shell find . -maxdepth 1 -mindepth 1 -type d | grep -v $(SED_EXCLUDE_FROM_SRC) | sed s/.\\///)
SOURCES = $(shell find $(SOURCE_DIRS) -name "*.cpp")
OBJFILES = $(patsubst %.cpp,%.o,$(SOURCES))
APPNAME = $(shell basename `pwd`)
CORE_INCLUDES = $(shell find ../../../libs/openFrameworks/ -type d)
CORE_INCLUDE_FLAGS = $(addprefix -I,$(CORE_INCLUDES))
INCLUDES = $(shell find ../../../libs/*/include -type d)
INCLUDES_FLAGS = $(addprefix -I,$(INCLUDES))
LIB_STATIC = $(shell ls ../../../libs/*/lib/$(LIBSPATH)/*.a | grep -v openFrameworksCompiled | sed "s/.*\\/lib\([^/]*\)\.a/-l\1/")
LIB_SHARED = $(shell ls ../../../libs/*/lib/$(LIBSPATH)/*.so | grep -v openFrameworksCompiled| sed "s/.*\\/lib\([^/]*\)\.so/-l\1/")

#LIB_PATHS_FLAGS = -L../../../libs/openFrameworksCompiled/lib/$(LIBSPATH)
LIB_PATHS_FLAGS = $(shell ls -d ../../../libs/*/lib/$(LIBSPATH) | sed "s/\(\.*\)/-L\1/")

CFLAGS = -Wall -fexceptions
CFLAGS += -I. 
CFLAGS += $(INCLUDES_FLAGS)
CFLAGS += $(CORE_INCLUDE_FLAGS)
CFLAGS += `pkg-config  gstreamer-0.10 gstreamer-video-0.10 gstreamer-base-0.10 libudev libavcodec libavformat libavutil --cflags`

LDFLAGS = $(LIB_PATHS_FLAGS) -Wl,-rpath=./libs

LIBS = $(LIB_SHARED)
LIBS += $(LIB_STATIC)
LIBS +=`pkg-config  gstreamer-0.10 gstreamer-video-0.10 gstreamer-base-0.10 libudev --libs`
LIBS += -lglut -lGL -lGLU -lasound

ifeq ($(findstring addons.make,$(wildcard *.make)),addons.make)
	ADDONS_INCLUDES = $(shell find ../../../addons/*/src/ -type d)
	ADDONS_INCLUDES += $(shell find ../../../addons/*/libs/ -type d)
	ADDONSCFLAGS = $(addprefix -I,$(ADDONS_INCLUDES))
	
	ADDONS_LIBS_STATICS = $(shell ls ../../../addons/*/libs/*/lib/$(LIBSPATH)/*.a)
	ADDONS_LIBS_SHARED = $(shell ls ../../../addons/*/libs/*/lib/$(LIBSPATH)/*.so)

	ADDONSLIBS = $(ADDONS_LIBS_STATICS)
	ADDONSLIBS += $(ADDONS_LIBS_SHARED)

	ADDONS = $(shell cat addons.make)
	ADDONS_REL_DIRS = $(addsuffix /src, $(ADDONS))
	ADDONS_LIBS_REL_DIRS = $(addsuffix /libs, $(ADDONS))
	ADDONS_DIRS = $(addprefix ../../../addons/, $(ADDONS_REL_DIRS) )
	ADDONS_LIBS_DIRS = $(addprefix ../../../addons/, $(ADDONS_LIBS_REL_DIRS) )
	ADDONS_SOURCES = $(shell find $(ADDONS_DIRS) -name "*.cpp")
	ADDONS_SOURCES += $(shell find $(ADDONS_LIBS_DIRS) -name "*.cpp" 2>/dev/null)
	ADDONS_OBJFILES = $(subst ../../../, ,$(patsubst %.cpp,%.o,$(ADDONS_SOURCES)))
endif



ifeq ($(findstring Debug,$(MAKECMDGOALS)),Debug)
	TARGET_CFLAGS = -g
	TARGET_LIBS = -lopenFrameworksDebug
	OBJ_OUTPUT = obj/Debug/
	TARGET_NAME = Debug
	TARGET = bin/$(APPNAME)_debug
	CLEANTARGET = CleanDebug
endif

ifeq ($(findstring Release,$(MAKECMDGOALS)),Release)
	TARGET_CFLAGS = $(COMPILER_OPTIMIZATION)
	TARGET_LIBS = -lopenFrameworks
	OBJ_OUTPUT = obj/Release/
	TARGET_NAME = Release
	TARGET = bin/$(APPNAME)
	CLEANTARGET = CleanRelease
endif

	# default to release
ifeq (,$(MAKECMDGOALS))
	TARGET_CFLAGS = $(COMPILER_OPTIMIZATION)
	TARGET_LIBS = -lopenFrameworks
	OBJ_OUTPUT = obj/Release/
	TARGET_NAME = Release
	TARGET = bin/$(APPNAME)
endif

ifeq ($(MAKECMDGOALS),depend-Release)
	OBJ_OUTPUT = obj/Release/
endif

ifeq ($(MAKECMDGOALS),depend-Debug)
	OBJ_OUTPUT = obj/Debug/
endif


ifeq ($(MAKECMDGOALS),clean)
	TARGET = bin/$(APPNAME)_debug bin/$(APPNAME)
endif


ifeq ($(TARGET_NAME),Release)
	OF_DEPEND = ../../../libs/openFrameworksCompiled/lib/$(LIBSPATH)/libopenFrameworks.a
else
	OF_DEPEND = ../../../libs/openFrameworksCompiled/lib/$(LIBSPATH)/libopenFrameworksDebug.a
endif

OBJS = $(addprefix $(OBJ_OUTPUT), $(OBJFILES))
DEPFILES = $(patsubst %.o,%.d,$(OBJS))

# addons
ifeq ($(findstring addons.make,$(wildcard *.make)),addons.make)
	ADDONS_OBJS = $(addprefix $(OBJ_OUTPUT), $(ADDONS_OBJFILES))
endif


.PHONY: Debug Release all after depend

Release: $(TARGET) after

Debug: $(TARGET) after

all: $(TARGET)
	make Release
	make Depend

depend: 
	make depend-Release 
	make depend-Debug

debugging_blah:
	@echo MSG: 
	@echo $(MSG)
	@echo source dirs:
	@echo $(SOURCE_DIRS)
	@echo sources:
	@echo $(SOURCES)
	@echo addons_sources:
	@echo $(ADDONS_SOURCES)	
	@echo obj_output:
	@echo $(OBJ_OUTPUT)	
	@echo objs:
	@echo $(OBJS)
	@echo addons_objs:
	@echo $(ADDONS_OBJS)
	@echo makecmdgoals:
	@echo $(MAKECMDGOALS)
	

# we need separate rules for Debug and Release because of different obj/ paths
depend-Release: $(DEPFILES)
depend-Debug: $(DEPFILES)

# This is the rule for creating the dependency files
#$(OBJ_OUTPUT)%.d : %.cpp
#	@echo " * "creating dependency file $@ for $<
#	@mkdir -p $(@D)
#	@# dependency creation flags
#	@# -MM: exclude system headers
#	@# -MT: change target name
#	@# -MF: write to file
#	@$(CXX) $(TARGET_CFLAGS) $(CFLAGS) $(ADDONSCFLAGS) $(USER_CFLAGS) -MM -MT $(patsubst %.d,%.o,$@) -MF $@ $<
	
#$(OBJ_OUTPUT)%.d : ../../../%.cpp
#	@echo " * "creating addon dependency file $@ for $<
#	@mkdir -p $(@D)
#	@# dependency creation flags
#	@# -MM: exclude system headers
#	@# -MT: change target name
#	@# -MF: write to file
#	@$(CXX) $(TARGET_CFLAGS) $(CFLAGS) $(ADDONSCFLAGS) $(USER_CFLAGS) -MM -MT $(patsubst %.d,%.o,$@) -MF $@ $<

#This rule does the compilation
#$(OBJS): $(SOURCES) $(DEPFILES)
$(OBJ_OUTPUT)%.o : ../../../%.cpp
	@echo " * "compiling addon object $@ from $<
	@mkdir -p $(@D)
	@# -MMD: update the dependency file
	$(CXX) $(TARGET_CFLAGS) $(CFLAGS) $(ADDONSCFLAGS) $(USER_CFLAGS) -MMD -o $(patsubst ../../../%.cpp,$(OBJ_OUTPUT)%.o,$<) -c $<

$(OBJ_OUTPUT)%.o : %.cpp 
	@echo " * "compiling object $@ from $< 
	@mkdir -p $(@D)
	@# -MMD: update the dependency file
	$(CXX) $(TARGET_CFLAGS) $(CFLAGS) $(ADDONSCFLAGS) $(USER_CFLAGS) -MMD -o $(patsubst %.cpp,$(OBJ_OUTPUT)%.o,$<) -c $<


$(TARGET) : $(OBJS) $(ADDONS_OBJS) $(OF_DEPEND)
	@echo linking $(TARGET)
	@echo "creating " $(TARGET)
	$(CXX) $(TARGET_CFLAGS) -o $@ $(OBJS) $(ADDONS_OBJS) $(CFLAGS) $(ADDONSCFLAGS) $(USER_CFLAGS) $(LDFLAGS) $(USER_LDFLAGS) $(TARGET_LIBS) $(LIBS) $(ADDONSLIBS) $(USER_LIBS)

.PHONY: clean CleanDebug CleanRelease
clean: 
	rm -Rf obj
	rm -f -v $(TARGET)
	rm -Rf -v bin/libs
	
$(CLEANTARGET):
	rm -Rf -v $(OBJ_OUTPUT)
	rm -f -v $(TARGET)


after:
	@cp -r ../../../export/$(LIBSPATH)/libs bin/
	@echo
	@echo "     compiling done"
	@echo "     to launch the application"	
	@echo
	@echo "     cd bin"
	@echo "     ./$(TARGET)"
	@echo
    

.PHONY: help
help:
 
	@echo 
	@echo openFrameworks universal makefile
	@echo
	@echo targets:
	@echo "make Debug:		builds the application with debug symbols"
	@echo "make Release:		builds the app with optimizations"
	@echo "make:			= make Release"
	@echo "make all:		= make Release"
	@echo "make CleanDebug:	cleans the Debug target"
	@echo "make CleanRelease:	cleans the Release target"
	@echo "make clean:		cleans everything"
	@echo
	@echo this should work with any OF app, just copy any example
	@echo change the name of the folder and it should compile
	@echo "only .cpp support, don't use .c files"
	@echo it will look for files in any folder inside the application
	@echo folder except that in the EXCLUDE_FROM_SOURCE variable.
	@echo "it doesn't autodetect include paths yet"
	@echo "add the include paths editing the var USER_CFLAGS"	
	@echo at the beginning of the makefile using the gcc syntax:
	@echo -Ipath
	@echo
	@echo to add addons to your application, edit the addons.make file
	@echo in this directory and add the names of the addons you want to
	@echo include
	@echo

# include dependencies
#ifeq (0,$(words $(findstring $(MAKECMDGOALS),$(NODEPS))))
#	MSG = \'$(findstring $(MAKECMDGOALS),$(NODEP))\' : depfiles match $(MAKECMDGOALS) $(NODEP)	
#	-include $(DEPFILES)
#else
#	MSG = \'$(findstring $(MAKECMDGOALS),$(NODEP))\' : depfiles included
#endif

-include $(DEPFILES)

