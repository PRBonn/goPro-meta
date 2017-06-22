cmake_minimum_required (VERSION 2.6)
project (img_gps_extractor)
# The version number and some more build stuff.
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
execute_process(COMMAND git describe --abbrev=8 --always --tags OUTPUT_VARIABLE GIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND date OUTPUT_VARIABLE DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git config user.name OUTPUT_VARIABLE GIT_USER OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git rev-parse --show-toplevel OUTPUT_VARIABLE PROJ_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)

# use c++
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/include/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
) 
# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

# add gpmf-parser include dirs
include_directories("${PROJ_ROOT}/extlib/gpmf-parser/")
include_directories("${PROJ_ROOT}/extlib/gpmf-parser/demo")
include_directories("${PROJ_ROOT}/extlib/gpmf-parser/include")

# add include to include dirs
include_directories("${PROJECT_SOURCE_DIR}/include")

# add executable for main app
file(GLOB CXXSRC
     ${PROJECT_SOURCE_DIR}/src/mp4_img_extractor.cpp
     ${PROJECT_SOURCE_DIR}/src/gpmf_to_yaml.cpp
     ${PROJECT_SOURCE_DIR}/src/main.cpp)
file(GLOB CSRC
     ${PROJ_ROOT}/extlib/gpmf-parser/GPMF_parser.c
     ${PROJ_ROOT}/extlib/gpmf-parser/demo/GPMF_mp4reader.c
     ${PROJ_ROOT}/extlib/gpmf-parser/demo/GPMF_print.c)
add_executable(img_gps_extractor ${CSRC} ${CXXSRC})

# link libraries
# add boost
find_package (Boost COMPONENTS system filesystem program_options REQUIRED)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  # message("BOOST INCLUDE: ${Boost_INCLUDE_DIRS}")
  target_link_libraries (img_gps_extractor ${Boost_LIBRARIES}
                                           ${Boost_SYSTEM_LIBRARY}
                                           ${Boost_FILESYSTEM_LIBRARY}
                                           ${Boost_PROGRAM_OPTIONS_LIBRARY})
  # message("BOOST LIB: ${Boost_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY}")
endif (Boost_FOUND)

# libyaml-cpp
find_package (yaml-cpp REQUIRED)
if (yaml-cpp_FOUND)
  include_directories(${YAML_CPP_INCLUDE_DIR})
  # message("YAML INCLUDE: ${YAML_CPP_INCLUDE_DIR}")
  target_link_libraries (img_gps_extractor ${YAML_CPP_LIBRARIES})
  # message("YAML LIB: ${YAML_CPP_LIBRARIES}")
  message("-- libyaml-cpp found! Version: ${yaml-cpp_VERSION}")
endif (yaml-cpp_FOUND)

# opencv
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  # message("OpenCV INCLUDE: ${OpenCV_INCLUDE_DIRS}")
  target_link_libraries (img_gps_extractor ${OpenCV_LIBRARIES})
  # message("OpenCV LIB: ${OpenCV_LIBRARIES}")
  message("-- OpenCV found! Version: ${OpenCV_VERSION}")
endif (OpenCV_FOUND)