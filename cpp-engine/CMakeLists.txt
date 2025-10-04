# --- Project Definition ---
# Explicitly enable both CXX (C++) and C languages.
cmake_minimum_required(VERSION 3.16)
project(llamaShieldEngine LANGUAGES CXX C)

# ===================================================================
# *** THE CRITICAL FIX #1: SET C++ STANDARD TO 20 ***
# ===================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ===================================================================

# --- Project Structure ---
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# --- Find Dependencies ---
find_package(Threads REQUIRED)
find_package(Numa) # Optional, for Linux

# ===================================================================
# Vendor Library Definition (xxhash)
# ===================================================================
add_library(xxhash_lib STATIC ${VENDOR_DIR}/xxhash/xxhash.c)
target_include_directories(xxhash_lib PUBLIC ${VENDOR_DIR}/xxhash)

# ===================================================================
# Main Executable Definition
# ===================================================================
add_executable(llamaShield_engine
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/BinaryFuseWrapper.cpp
    ${SRC_DIR}/numa_optimized_filter.cpp
)
add_dependencies(llamaShield_engine xxhash_lib)

# ===================================================================
# Properties for the Main Executable
# ===================================================================

# --- Include Directories ---
target_include_directories(llamaShield_engine PUBLIC
    ${INCLUDE_DIR}
    ${VENDOR_DIR}/binfuse/include
    ${VENDOR_DIR}/binfuse/ext/xor_singleheader/include
    ${VENDOR_DIR}/binfuse/ext/mio/include
    ${VENDOR_DIR}/fastfilter_cpp/src
    ${VENDOR_DIR}/concurrentqueue
)

# --- Compile Options ---
if(MSVC)
  # Explicitly tell MSVC to use the C++20 standard
  target_compile_options(llamaShield_engine PRIVATE /std:c++20)
  # *** THE CRITICAL FIX #2: Use Generator Expressions for build-type flags ***
  target_compile_options(llamaShield_engine PRIVATE "$<$<CONFIG:Release>:/O2>")
else()
  target_compile_options(llamaShield_engine PRIVATE -O3 -march=native -funroll-loops)
endif()

# --- Library Linking ---
target_link_libraries(llamaShield_engine PRIVATE
    Threads::Threads
    xxhash_lib
)

if(Numa_FOUND)
    target_link_libraries(llamaShield_engine PRIVATE Numa::Numa)
endif()