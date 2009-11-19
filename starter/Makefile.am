lib_LIBRARIES = libstarter.a

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = starter.pc

EXTRA_DIST = starter.pc.in

INCLUDES= $(OPENCV_CFLAGS)

libstarter_a_SOURCES= \
geometry/affinity.cpp\
geometry/homography.cpp\
geometry/projection_matrix.cpp\
image/mcv.cpp\
image/pyrimage.cpp\
math/linear_algebra.cpp\
math/linear_systems.cpp\
math/mcvm.cpp\
math/polynom_solver.cpp\
optimization/ls_minimizer2.cpp\
optimization/ls_step_solver.cpp\
general/general.h\
geometry/affinity.h\
geometry/homography.h\
geometry/projection_matrix.h\
image/mcv.h\
image/pyrimage.h\
math/linear_algebra.h\
math/linear_systems.h\
math/mcvm.h\
math/polynom_solver.h\
math/robust_estimators.h\
math/growmat.cpp \
math/growmat.h \
optimization/ls_minimizer2.h\
optimization/ls_step_solver.h

nobase_pkginclude_HEADERS = general/general.h\
geometry/affinity.h\
geometry/homography.h\
geometry/projection_matrix.h\
image/mcv.h\
image/pyrimage.h\
math/linear_algebra.h\
math/linear_systems.h\
math/mcvm.h\
math/polynom_solver.h\
math/robust_estimators.h\
optimization/ls_minimizer.h\
optimization/ls_minimizer2.h\
optimization/ls_step_solver.h\
optimization/observation_types.h\
optimization/prosac_function_types.h \
starter.h
