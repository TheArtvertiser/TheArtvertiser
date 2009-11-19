#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(bazar, 1.3.1, julien.pilet@epfl.ch)
AM_INIT_AUTOMAKE


AC_CONFIG_SRCDIR([starter/starter.h])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
AX_CHECK_GLUT
AC_CHECK_LIB(GLEW, glewInit, have_glew=yes, have_glew=no, $GLUT_LIBS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h stdlib.h])
AC_CHECK_HEADER([GL/glew.h], [], have_glew=no)

if test "x$have_glew" = "xyes"; then
	GLEW_LIBS="-lGLEW $GL_LIBS"
	AC_DEFINE(HAVE_GLEW, 1, [defined if the GL Extension Wrangler GLEW library is available])
else
	GLEW_LIBS="$GL_LIBS"
	AC_MSG_WARN([GL Extension Wrangler library not found.  Get it from http://glew.sourceforge.net])
fi
AC_SUBST(GLEW_LIBS)

if test "x$no_gl" != "xyes"; then
	AC_DEFINE(HAVE_GL, 1, [defined if OpenGL is available])
fi

if test x$have_glew = xyes -a x$no_glut != xyes; then
	HAVE_FULLGL="yes"
else
	HAVE_FULLGL="no"
fi
AM_CONDITIONAL(FULLGL, test x$HAVE_FULLGL=xyes)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

PKG_CHECK_MODULES(OPENCV, opencv >= 0.9.6)
AC_SUBST(OPENCV_CFLAGS)
AC_SUBST(OPENCV_LIBS)

# Checks for library functions.
AC_CHECK_FUNCS([floor memset pow sqrt])
AC_CONFIG_FILES([
Makefile
garfeild/Makefile
garfeild/garfeild.pc
starter/Makefile
starter/starter.pc
doc/Doxyfile
doc/Makefile
artvertiser/Makefile
])
AC_OUTPUT
AC_MSG_NOTICE([Build OpenGL example: $HAVE_FULLGL])
