#Change PROJECT_NAME to the name of your project
PROJECT(parabolicMorpholgy)

# set the name of the input image used to run the test
SET(INPUT_IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/images/cthead1.png)
SET(INPUT_IMAGE3D ${CMAKE_CURRENT_SOURCE_DIR}/images/bunnyPadded.nrrd)

#include some macros from another file...
INCLUDE(IJMacros.txt)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

#The following lines are required to use Dart
ENABLE_TESTING()
INCLUDE(Dart)
FIND_PROGRAM(ITK_TEST_DRIVER itkTestDriver)
SET(TEST_COMMAND ${ITK_TEST_DRIVER} --add-before-env PATH ${CMAKE_CURRENT_BINARY_DIR})

#Declare any external dependencies that your project may have here.
#examples include: ITK, VTK, JPEG, PNG, OpenGL, ZLIB, Perl, Java
#If you're not sure what name to use, look in the Modules directory of your
#cmake install and check that a file named Find(Package).cmake exists
SET(Required_Packages
  ITK
)

#this foreach loads all of the packages that you specified as required.
#It shouldn't need to be modified.
FOREACH(Package ${Required_Packages})
  LOADPACKAGE(${Package})
ENDFOREACH(Package)

#Set any libraries that your project depends on.
#examples: ITKCommon, VTKRendering, etc
#SET(Libraries
#  ITKCommon
#  ITKBasicFilters
#  ITKIO
#  ITKAlgorithms
#)

# ITK4 version
SET(Libraries
${ITK_LIBRARIES}
)

# install devel files
OPTION(INSTALL_DEVEL_FILES "Install C++ headers" ON)
IF(INSTALL_DEVEL_FILES)
FILE(GLOB develFiles *.h *.txx) 
FOREACH(f ${develFiles})
  INSTALL_FILES(/include/InsightToolkit/BasicFilters FILES ${f})
ENDFOREACH(f)
ENDIF(INSTALL_DEVEL_FILES)



# option for wrapping
OPTION(BUILD_WRAPPERS "Wrap library" OFF)
IF(BUILD_WRAPPERS)
   SUBDIRS(Wrapping)
ENDIF(BUILD_WRAPPERS)
   
   

#the following block of code is an example of how to build an executable in
#cmake.  Unmodified, it will add an executable called "MyExe" to the project.
#MyExe will be built using the files MyClass.h and MyClass.cxx, and it will
#be linked to all the libraries you specified above. 
#You can build more than one executable per project

IF(BUILD_TESTING)

FOREACH(CurrentExe "check" "testOpen" "testClose" "testSpacing" "testSharpen" "testDT" "perfDT" "perfDT3D" "mkSpot" "erodeBinary2D" "testErodeBinary" "testDilateBinary" "testOpenBinary" "testCloseBinary" "dilateBinary3D" "testErode" "testDilate")
ADD_EXECUTABLE(${CurrentExe} ${CurrentExe}.cxx)
TARGET_LINK_LIBRARIES(${CurrentExe} ${Libraries})
ENDFOREACH(CurrentExe)

ENDIF(BUILD_TESTING)

#the following line is an example of how to add a test to your project.
#Testname is the title for this particular test.  ExecutableToRun is the
#program which will be running this test.  It can either be a part of this
#project or an external executable.  After that list any args that are needed
#for this test.  Include as many tests as you like.  If your project doesn't have
#any tests you can comment out or delete the following line.
# ADD_TEST(Testname ExecutableToRun arg1 arg2 arg3)

ADD_TEST(Test ${TEST_COMMAND}
   check ${INPUT_IMAGE} out.png
   --compare out.png ${CMAKE_CURRENT_SOURCE_DIR}/images/test.png
)

# default scale - 1
ADD_TEST(Erode2Da ${TEST_COMMAND}
   testErode ${INPUT_IMAGE} outEInta.png outECPa.png 
   --compare outEInta.png outECPa.png
)

ADD_TEST(Dilate2Da ${TEST_COMMAND}
   testDilate ${INPUT_IMAGE} outDInta.png outDCPa.png
   --compare outDInta.png outDCPa.png
)

# small scale
ADD_TEST(Erode2Db ${TEST_COMMAND}
   testErode ${INPUT_IMAGE} outEIntb.png outECPb.png 0.2
   --compare outEIntb.png outECPb.png
)


ADD_TEST(Dilate2Db ${TEST_COMMAND}
   testDilate ${INPUT_IMAGE} outDIntb.png outDCPb.png 0.2
   --compare outDIntb.png outDCPb.png
)


# large scale
ADD_TEST(Erode2Dc ${TEST_COMMAND}
   testErode ${INPUT_IMAGE} outEIntc.png outECPc.png 5.0
   --compare outEIntc.png outECPc.png
)

ADD_TEST(Dilate2Dc ${TEST_COMMAND}
   testDilate ${INPUT_IMAGE} outDIntc.png outDCPc.png 5.0
   --compare outDIntc.png outDCPc.png
)

ADD_TEST(Open ${TEST_COMMAND}
   testOpen ${INPUT_IMAGE} openCP.png openInt.png
   --compare openCP.png ${CMAKE_CURRENT_SOURCE_DIR}/images/open.png
   --compare openCP.png openInt.png
)
ADD_TEST(Spacing ${TEST_COMMAND}
   testSpacing ${INPUT_IMAGE} o1.png o2.png
   --compare o1.png o2.png
)

ADD_TEST(Sharpen1 ${TEST_COMMAND}
   testSharpen 1 sharp1.mha sharp1.txt
)

ADD_TEST(Sharpen2 ${TEST_COMMAND}
   testSharpen 2 sharp2.mha sharp2.txt
)

ADD_TEST(Sharpen3 ${TEST_COMMAND}
   testSharpen 3 sharp3.mha sharp3.txt
)
ADD_TEST(Sharpen10 ${TEST_COMMAND}
   testSharpen 10 sharp10.mha sharp10.txt
)
ADD_TEST(Sharpen100 ${TEST_COMMAND}
   testSharpen 100 sharp100.mha sharp100.txt
)

ADD_TEST(Dist1 ${TEST_COMMAND}
   testDT ${INPUT_IMAGE} 100 0 mask.png dist1.mha
)

ADD_TEST(Dist2 ${TEST_COMMAND}
   testDT ${INPUT_IMAGE} 100 255 mask.png dist2.mha
)

ADD_TEST(Perf ${TEST_COMMAND}
   perfDT ${INPUT_IMAGE} 100 255 mask.png dist3.mha dist4.mha dist5.mha
   --compare dist3.mha dist4.mha
)

ADD_TEST(Perf3D ${TEST_COMMAND}
   perfDT3D ${INPUT_IMAGE3D} 100 255 bmask.nii.gz distA.nii.gz distB.nii.gz distC.nii.gz
)
