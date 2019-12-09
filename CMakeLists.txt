cmake_minimum_required(VERSION 2.8.9)

include(CheckCSourceRuns)
include(CheckTypeSize)
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)

project(ecli C)

# Version Info
set(ECLI_VER_MAJOR 0)
set(ECLI_VER_MINOR 1)

check_include_file("string.h" HAVE_STRING_H)
check_include_file("memory.h" HAVE_MEMORY_H)

# Check for size_t
check_type_size(size_t SIZE_T)
if (NOT ${HAVE_SIZE_T})
  check_c_source_runs("
  #ifdef _WIN32
  # include <BaseTsd.h>
  typedef SIZE_T size_t;
  #else
  # include <sys/types.h>
  #endif
  int main() {
    size_t test = -1;
    return test>0 ? 0 : 1;
  }
  " HAVE_SSIZE_T_BASETSD)
endif()
check_type_size(ssize_t SSIZE_T)
if (NOT ${HAVE_SSIZE_T})
  check_c_source_runs("
  #ifdef _WIN32
  # include <BaseTsd.h>
  typedef SSIZE_T ssize_t;
  #else
  # include <sys/types.h>
  #endif
  int main() {
    ssize_t test = -1;
    return test<0 ? 0 : 1;
  }
  " HAVE_SSIZE_T_BASETSD)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file("include/config.h.in" "config.h")

include_directories(include)
file(GLOB SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
