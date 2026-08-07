set(SRC_DIR "${CMAKE_SOURCE_DIR}/Core/src")

set(SRC_ADDS "${SRC_DIR}/core_exceptions.cpp"
"${SRC_DIR}/separator.cpp"
"${SRC_DIR}/tokenizer.cpp"
"${SRC_DIR}/name_table.cpp"
"${SRC_DIR}/llvm_value.cpp"
"${SRC_DIR}/source_manager.cpp"
"${SRC_DIR}/source_location_ul.cpp"
"${SRC_DIR}/policy.cpp"
"${SRC_DIR}/file_module.cpp"
"${SRC_DIR}/ulstring.cpp"
)

add_library (CoreStaticLib "${SRC_ADDS}" )
target_include_directories(CoreStaticLib PUBLIC "${CMAKE_SOURCE_DIR}/Core/include")

add_executable(ul_test "${SRC_ADDS}" "${SRC_DIR}/test.cpp" )
target_include_directories(ul_test PUBLIC "${CMAKE_SOURCE_DIR}/Core/include")