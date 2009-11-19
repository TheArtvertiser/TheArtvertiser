lib_LIBRARIES = libgarfeild.a

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = garfeild.pc

EXTRA_DIST = garfeild.pc.in

INCLUDES= $(OPENCV_CFLAGS) -I@top_srcdir@/starter

libgarfeild_a_SOURCES= \
keypoints/keypoint_orientation_corrector.cpp \
keypoints/yape.cpp \
viewsets/affine_image_generator.cpp \
viewsets/image_class_example.cpp \
viewsets/image_classification_forest.cpp \
viewsets/image_classification_node.cpp \
viewsets/image_classification_tree.cpp \
viewsets/image_classifier.cpp \
viewsets/object_view.cpp \
viewsets/planar_object_recognizer.cpp \
keypoints/keypoint.h \
keypoints/keypoint_match.h \
keypoints/keypoint_orientation_corrector.h \
keypoints/yape.h \
viewsets/affine_image_generator.h \
viewsets/example_generator.h \
viewsets/image_class_example.h \
viewsets/image_classification_forest.h \
viewsets/image_classification_node.h \
viewsets/image_classification_tree.h \
viewsets/image_classifier.h \
viewsets/image_object_point_match.h \
viewsets/object_keypoint.h \
viewsets/object_view.h \
viewsets/planar_object_recognizer.h \
garfeild.h \
calib/CamAugmentation.cpp \
calib/CamCalibration.cpp \
calib/CamCalibration.h \
calib/CamAugmentation.h \
calib/gradient.cpp \
calib/camera.cpp \
calib/camera.h \
calib/matvec.h \
calib/matvec.cpp \
lightcalib/ipltexture.cpp \
lightcalib/ipltexture.h \
lightcalib/lightcalib.h \
lightcalib/lightcollector.cpp \
lightcalib/lightcollector.h \
lightcalib/lightmap.cpp \
lightcalib/lightmap.h \
lightcalib/tri.cpp

nobase_pkginclude_HEADERS =  \
keypoints/keypoint.h \
keypoints/keypoint_match.h \
keypoints/keypoint_orientation_corrector.h \
keypoints/yape.h \
viewsets/affine_image_generator.h \
viewsets/example_generator.h \
viewsets/image_class_example.h \
viewsets/image_classification_forest.h \
viewsets/image_classification_node.h \
viewsets/image_classification_tree.h \
viewsets/image_classifier.h \
viewsets/image_object_point_match.h \
viewsets/object_keypoint.h \
viewsets/object_view.h \
viewsets/planar_object_recognizer.h \
calib/CamCalibration.h \
calib/CamAugmentation.h \
calib/camera.h \
calib/matvec.h \
garfeild.h
