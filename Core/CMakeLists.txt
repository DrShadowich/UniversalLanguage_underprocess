project(Core)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/Core/src")

add_library (CoreStaticLib  
"${SRC_DIR}/core_exceptions.cpp"
"${SRC_DIR}/commentaries.cpp"
"${SRC_DIR}/separator.cpp"
"${SRC_DIR}/tokenizer.cpp"
"${SRC_DIR}/simple_variable.cpp"
"${SRC_DIR}/name_table.cpp"
)
target_include_directories(CoreStaticLib PUBLIC "${CMAKE_SOURCE_DIR}/Core/include")

add_executable(ul_test 
"${SRC_DIR}/core_exceptions.cpp"
"${SRC_DIR}/commentaries.cpp" 
"${SRC_DIR}/test.cpp"
"${SRC_DIR}/separator.cpp"
"${SRC_DIR}/tokenizer.cpp"
"${SRC_DIR}/simple_variable.cpp"
"${SRC_DIR}/name_table.cpp"
)
target_include_directories(ul_test PUBLIC "${CMAKE_SOURCE_DIR}/Core/include")
