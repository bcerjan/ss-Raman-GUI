# Project Name and language:
PROJECT(ss-sers-control LANGUAGES C)

CMAKE_MINIMUM_REQUIRED(VERSION 3.19)

# We need PkgConfig to detect GTK headers/libraries
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK REQUIRED gtk+-3.0)

# Now tell CMake to use said libraries and whatnot:
#INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS})
#LINK_DIRECTORIES(${GTK_LIBRARY_DIRS})

# Add additional flags:
ADD_DEFINITIONS(${GTK_CFLAGS_OTHER})

# equivalent of -rdyanmic, but that doesn't exist here:
SET(GCC_DYNAMIC_LINK_FLAGS "-Wl,--export-all-symbols")

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
SET (COMMON_SRCS
  ${CMAKE_SOURCE_DIR}/src/main.c
  ${CMAKE_SOURCE_DIR}/src/waveform_gen.c
)

# Put our executable in the root directory:
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add our executable:
ADD_EXECUTABLE(app ${COMMON_SRCS})

# Specify to use our custom linker flags:
TARGET_LINK_OPTIONS(app PUBLIC "${GCC_DYNAMIC_LINK_FLAGS}")

# Link GTK to the target:
TARGET_LINK_LIBRARIES(app PUBLIC ${GTK_LIBRARIES})

# And waveform generator:
TARGET_LINK_LIBRARIES(app PUBLIC ${DAX_LIB})
