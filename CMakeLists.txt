# Project Name and language:
PROJECT(ss-sers-control LANGUAGES C)

CMAKE_MINIMUM_REQUIRED(VERSION 3.19)

# We need PkgConfig to detect GTK headers/libraries
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK REQUIRED gtk+-3.0)

# Add additional flags:
ADD_DEFINITIONS(${GTK_CFLAGS_OTHER})

# equivalent of -rdyanmic, but that doesn't exist here:
SET(GCC_DYNAMIC_LINK_FLAGS "-Wl,--export-all-symbols")

# Need to find threading library and apply flags:
SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)

# Include header files from include directories
SET(MAIN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES(
  ${GTK_INCLUDE_DIRS}
  #${MAIN_INCLUDE_DIR}/fftw
  ${MAIN_INCLUDE_DIR}/wavepond
  ${MAIN_INCLUDE_DIR}/app
)

# Add libraries
SET(MAIN_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/lib")
find_library(DAX_LIB dax22000_lib_DLL64 HINTS ${MAIN_LIBRARY_DIR}/wavepond)


# Source Files:
SET(MAIN_SRC_DIR "${CMAKE_SOURCE_DIR}/src")

SET (COMMON_SRCS
  ${MAIN_SRC_DIR}/main.c
  ${MAIN_SRC_DIR}/waveform_gen.c
  ${MAIN_SRC_DIR}/acquire_data.c
  ${MAIN_SRC_DIR}/data_output.c
  ${MAIN_SRC_DIR}/fft_functions.c
)

SET (TEST_SRCS
  ${MAIN_SRC_DIR}/test.c
)

# Subdirectory for generator:
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/generators)

# Put our executable in the root directory:
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Add our executable:
ADD_EXECUTABLE(app ${COMMON_SRCS} pn_code.h)
ADD_EXECUTABLE(test ${TEST_SRCS})

# Specify to use our custom linker flags:
TARGET_LINK_OPTIONS(app PUBLIC ${GCC_DYNAMIC_LINK_FLAGS})
TARGET_LINK_OPTIONS(test PUBLIC ${GCC_DYNAMIC_LINK_FLAGS})

# Turn on more warnings:
TARGET_COMPILE_OPTIONS(app PRIVATE -Wall)
TARGET_COMPILE_OPTIONS(test PRIVATE -Wall)

# Link GTK to the target:
TARGET_LINK_LIBRARIES(app PUBLIC ${GTK_LIBRARIES})
TARGET_LINK_LIBRARIES(test PUBLIC ${GTK_LIBRARIES})

# And waveform generator:
TARGET_LINK_LIBRARIES(app PUBLIC ${DAX_LIB})

# Add math library:
TARGET_LINK_LIBRARIES(app PUBLIC m)
TARGET_LINK_LIBRARIES(test PUBLIC m)

# Add FFTW Library:
TARGET_LINK_LIBRARIES(app PUBLIC fftw3)
TARGET_LINK_LIBRARIES(test PUBLIC fftw3)

# Add pthreading:
TARGET_LINK_LIBRARIES(app PRIVATE Threads::Threads)
