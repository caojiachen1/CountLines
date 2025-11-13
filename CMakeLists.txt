cmake_minimum_required(VERSION 3.10)
project(CountLines VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler-specific options for optimization and warnings
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -march=native")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /O2")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
endif()

# Platform-specific libraries
if(WIN32)
    # Windows specific libraries
    set(PLATFORM_LIBS ws2_32)
elseif(UNIX)
    # Unix/Linux specific libraries
    set(PLATFORM_LIBS "")
endif()

# Source files
set(SOURCES
    src/main.c
    src/countlines.c
    src/webserver.c
)

# Header files
set(HEADERS
    src/countlines.h
    src/webserver.h
)

# Create executable
add_executable(countlines ${SOURCES} ${HEADERS})

# Link platform-specific libraries
target_link_libraries(countlines ${PLATFORM_LIBS})

# Include directories
target_include_directories(countlines PRIVATE src)

# Install target
install(TARGETS countlines DESTINATION bin)

# Set output directory
set_target_properties(countlines PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)