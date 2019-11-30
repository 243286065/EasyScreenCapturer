cmake_minimum_required (VERSION 2.8)
project(EasyScreenCapture)
#aux_source_directory(src DIR_SRCS)

# set output path
SET(PROJECT_OUTPUT_DIR ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR})
SET(PROJECT_INCLUDE_DIR ${PROJECT_OUTPUT_DIR}/include)
MESSAGE("-- output path:  ${PROJECT_OUTPUT_DIR} ")

# mkdir directory
file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR}) 
FILE(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)
FILE(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)

# set the target path
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(LIBS)

IF (WIN32)
    add_definitions(-DOS_WIN=1)
    set(
        PaltformSrc
        ${PROJECT_SOURCE_DIR}/src/EasyScreenCapturerWin.h
        ${PROJECT_SOURCE_DIR}/src/EasyScreenCapturerWin.cpp
    )
ELSEIF (UNIX)
    add_definitions(-DOS_UNIX=1)
    set(
        PaltformSrc
        ${PROJECT_SOURCE_DIR}/src/EasyScreenCapturerLinux.h
        ${PROJECT_SOURCE_DIR}/src/EasyScreenCapturerLinux.cpp
    )
    set(LIBS X11)
ELSE ()
    set(
        PaltformSrc
        ${PROJECT_SOURCE_DIR}/src/EasyScreenCapturerFaker.h
    )
ENDIF ()

include_directories(${PROJECT_SOURCE_DIR}/src/)

set(
    Src
    ${PROJECT_SOURCE_DIR}/src/EasyScreenCapturer.h
    ${PROJECT_SOURCE_DIR}/src/EasyScreenCapturer.cpp
    ${PaltformSrc}
)

# 指定生成目标
add_library(${PROJECT_NAME} ${Src})

SET(
    demoSrc
    ${PROJECT_SOURCE_DIR}/test/demo.cpp
)

SET(
    LINK_DIR
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

LINK_DIRECTORIES(${LINK_DIR})

if(${NO_BUILD_DEMO})
else()
    ADD_EXECUTABLE(demo ${demoSrc})
    TARGET_LINK_LIBRARIES(demo ${PROJECT_NAME} ${LIBS})
endif()