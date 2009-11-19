
if FULLGL
openglprogs=multigl
else
openglprogs=
endif

noinst_PROGRAMS=artvertiser $(openglprogs)

artvertiser_CXXFLAGS=@OPENCV_CFLAGS@ -I@top_srcdir@/garfeild -I@top_srcdir@/starter
artvertiser_LDADD=@OPENCV_LIBS@ ../../garfeild/libgarfeild.a ../../starter/libstarter.a \
		 @GLEW_LIBS@
artvertiser_SOURCES=artvertiser.cpp calibmodel.cpp calibmodel.h \
		 multigrab.h multigrab.cpp

multigl_CXXFLAGS=@OPENCV_CFLAGS@ -I@top_srcdir@/garfeild -I@top_srcdir@/starter
multigl_LDADD=@OPENCV_LIBS@ ../../garfeild/libgarfeild.a ../../starter/libstarter.a \
		 @GLEW_LIBS@ @GLUT_LIBS@
multigl_SOURCES=multigl.cpp calibmodel.cpp calibmodel.h \
		 multigrab.h multigrab.cpp
