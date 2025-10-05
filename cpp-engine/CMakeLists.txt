cmake_minimum_required(VERSION 3.16)
project(llamaShieldEngine LANGUAGES CXX C)

# -------------------------
# C++ Standard
# -------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Paths
# -------------------------
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# -------------------------
# Find base packages
# -------------------------
find_package(Threads REQUIRED)
find_package(Numa) # optional; will not fail if absent

# -------------------------
# Python and pybind11 setup - FIXED VERSION
# -------------------------
# First find Python with both Development and Interpreter components
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# -------------------------
# Use vendored pybind11 if present, otherwise require it
# -------------------------
if(EXISTS "${VENDOR_DIR}/pybind11/CMakeLists.txt")
    message(STATUS "Using vendored pybind11 from ${VENDOR_DIR}/pybind11")
    
    # Set Python locations before including pybind11
    set(Python_EXECUTABLE ${Python_EXECUTABLE})
    set(Python_INCLUDE_DIRS ${Python_INCLUDE_DIRS})
    set(Python_LIBRARIES ${Python_LIBRARIES})
    
    add_subdirectory(${VENDOR_DIR}/pybind11)
else()
    find_package(pybind11 REQUIRED) # fallback: requires pybind11 installed
endif()

# -------------------------
# Locate xxHash source file (try a couple of common paths)
# -------------------------
set(XXHASH_SRC ${VENDOR_DIR}/xxhash/xxhash.c)
if(NOT EXISTS "${XXHASH_SRC}")
    set(XXHASH_SRC ${VENDOR_DIR}/xxhash/src/xxhash.c)
endif()

if(NOT EXISTS "${XXHASH_SRC}")
    message(FATAL_ERROR "xxHash source not found. Expected at ${VENDOR_DIR}/xxhash/xxhash.c or ${VENDOR_DIR}/xxhash/src/xxhash.c")
endif()

# -------------------------
# Build xxhash static library
# -------------------------
add_library(xxhash_lib STATIC ${XXHASH_SRC})
target_include_directories(xxhash_lib PUBLIC ${VENDOR_DIR}/xxhash)

# -------------------------
# Core static library (compile common non-main sources once)
# -------------------------
set(CORE_SOURCES
    ${SRC_DIR}/BinaryFuseWrapper.cpp
    ${SRC_DIR}/MortonFilterWrapper.cpp
    ${SRC_DIR}/numa_optimized_filter.cpp
    # Add other core sources here (do NOT add main.cpp or python bindings here)
)

add_library(llamaShield_core STATIC ${CORE_SOURCES})

target_include_directories(llamaShield_core PUBLIC
    ${INCLUDE_DIR}
    ${VENDOR_DIR}/binfuse/include
    ${VENDOR_DIR}/binfuse/ext/xor_singleheader/include
    ${VENDOR_DIR}/binfuse/ext/mio/include
    ${VENDOR_DIR}/fastfilter_cpp/src
    ${VENDOR_DIR}/concurrentqueue
    ${VENDOR_DIR}/fastfilter_cpp/src/morton
    ${VENDOR_DIR}/xxhash         # so xxh3.h resolves if it's top-level
)

# Compile options: MSVC / GCC/Clang
if(MSVC)
    target_compile_options(llamaShield_core PRIVATE /std:c++20 $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(llamaShield_core PRIVATE -O3 -march=native -funroll-loops)
endif()

# Link core to xxhash and threads and optionally NUMA
target_link_libraries(llamaShield_core PRIVATE Threads::Threads xxhash_lib)
if(Numa_FOUND)
    target_link_libraries(llamaShield_core PRIVATE Numa::Numa)
endif()

# Ensure library build order (explicit)
add_dependencies(llamaShield_core xxhash_lib)

# -------------------------
# Executable
# -------------------------
add_executable(llamaShield_engine ${SRC_DIR}/main.cpp)
target_link_libraries(llamaShield_engine PRIVATE llamaShield_core)

# Propagate include directories (optional, already available via library)
target_include_directories(llamaShield_engine PRIVATE ${INCLUDE_DIR})

# -------------------------
# Python extension (pybind11) - FIXED VERSION
# -------------------------
# Name it differently than exe to avoid conflicts.
pybind11_add_module(llamashield_py
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings/python_bindings.cpp
)

# Link the same core library into the python module (no recompilation of core)
target_link_libraries(llamashield_py PRIVATE llamaShield_core)

# Ensure core builds first
add_dependencies(llamashield_py llamaShield_core)

# Make sure python module can find includes if needed (optional)
target_include_directories(llamashield_py PRIVATE ${INCLUDE_DIR} ${VENDOR_DIR}/xxhash)

# Explicitly set Python include directories and libraries for the module
target_include_directories(llamashield_py PRIVATE ${Python_INCLUDE_DIRS})
target_link_libraries(llamashield_py PRIVATE ${Python_LIBRARIES})

# -------------------------
# Install rules (optional)
# -------------------------
install(TARGETS llamaShield_engine RUNTIME DESTINATION bin)
install(TARGETS llamashield_py LIBRARY DESTINATION python)

# -------------------------
# Helpful status messages
# -------------------------
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Python executable: ${Python_EXECUTABLE}")
message(STATUS "Python include dirs: ${Python_INCLUDE_DIRS}")
message(STATUS "Python libraries: ${Python_LIBRARIES}")
if(Numa_FOUND)
    message(STATUS "NUMA support: found")
else()
    message(STATUS "NUMA support: not found (NUMA features disabled)")
endif()