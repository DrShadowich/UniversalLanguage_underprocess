cmake_minimum_required (VERSION 3.20)
project ("UniversalLanguage")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "D:/LLVMc/clang+llvm-22.1.0-x86_64-pc-windows-msvc/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory ("Core")
add_subdirectory("Language")

llvm_map_components_to_libnames(llvm_libs core bitwriter)

target_link_libraries(ul PRIVATE ${llvm_libs})
target_link_libraries(ul PRIVATE CoreStaticLib)

set_target_properties(ul PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


