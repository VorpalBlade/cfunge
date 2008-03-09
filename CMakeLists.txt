PROJECT(CFUNGE08)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
MARK_AS_ADVANCED(
        CMAKE_BACKWARDS_COMPATIBILITY
)


#if you don't want the full compiler output, remove the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -Wextra -Wformat=2)

#list all source files here
FILE(GLOB CFUNGE08_SOURCES RELATIVE ${CFUNGE08_SOURCE_DIR} src/*.c)
ADD_EXECUTABLE(cfunge08 ${CFUNGE08_SOURCES})


#need to link to some other libraries ? just add them here
#TARGET_LINK_LIBRARIES(cfunge08 png jpeg)
