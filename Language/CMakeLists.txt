set(LANGUAGE_DIR "${CMAKE_SOURCE_DIR}/Language")
set(SRC_DIR "${LANGUAGE_DIR}/src")
set(TOKENS_INC_DIR	"${LANGUAGE_DIR}/include/tokens")
set(LLVMIR_DIR "${LANGUAGE_DIR}/include/generation_of_llvm")
set(CORE_INCLUDE "${CMAKE_SOURCE_DIR}/Core/include")

set(SRC_FILES 
"${SRC_DIR}/token_info.cpp"
"${SRC_DIR}/token_type.cpp"
"${SRC_DIR}/main.cpp"
"${SRC_DIR}/lexer.cpp"
"${SRC_DIR}/parser.cpp"
"${SRC_DIR}/generating_visitors.cpp"
"${SRC_DIR}/code_generator.cpp"
"${SRC_DIR}/dictionaries.cpp"
"${SRC_DIR}/llvm_union_info.cpp"
"${SRC_DIR}/function_names_memory.cpp"
"${SRC_DIR}/function_name_info.cpp"
"${SRC_DIR}/expression_info.cpp"
"${SRC_DIR}/statement_info.cpp"
"${TOKENS_INC_DIR}/stmt_expr_cast.h"
"${SRC_DIR}/stmt_expr_cast.cpp"
)

add_executable(ul ${SRC_FILES} ${TOKENS_INC_DIR} ${LLVMIR_DIR} ${TOKENS_SRC_DIR})

target_include_directories(ul PUBLIC ${LLVMIR_DIR})
target_include_directories(ul PUBLIC ${CORE_INCLUDE})
target_include_directories(ul PUBLIC ${TOKENS_INC_DIR})