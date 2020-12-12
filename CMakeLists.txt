# Project Name and language:
PROJECT(ss-sers-control C)

CMAKE_MINIMUM_REQUIRED(VERSION 3.19)

# We need PkgConfig to detect GTK headers/libraries
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK REQUIRED gtk+-3.0)

# Now tell CMake to use said libraries and whatnot:
INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK_LIBRARY_DIRS})

# Add additional flags:
ADD_DEFINITIONS(${GTK_CFLAGS_OTHER})

# equivalent of -rdyanmic, but that doesn't exist here:
SET(GCC_DYNAMIC_LINK_FLAGS "-Wl,--export-all-symbols")

# Put our executable in the root directory:
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add our executable:
ADD_EXECUTABLE(app ./src/main.c)

# Specify to use our custom linker flags:
TARGET_LINK_OPTIONS(app PUBLIC "${GCC_DYNAMIC_LINK_FLAGS}")

# Link to the target:
TARGET_LINK_LIBRARIES(app ${GTK_LIBRARIES})
