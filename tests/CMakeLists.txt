# SPDX-FileCopyrightText: 2025 Andy Curtis <contactandyc@gmail.com>
# SPDX-License-Identifier: Apache-2.0

# CMakeLists.txt for tests
cmake_minimum_required(VERSION 3.20)

project(a_json_schema_builder_library_tests LANGUAGES C)

set(A_BUILD_VARIANT "debug" CACHE STRING
    "Variant to link via a_json_schema_builder_library::a_json_schema_builder_library (debug|memory|static|shared)")
set_property(CACHE A_BUILD_VARIANT PROPERTY STRINGS debug memory static shared)

option(A_ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

find_library(M_LIB m)

# ---- Test executables ----
set(TEST_EXECUTABLES "")
add_executable(test_ajsb  src/test_ajsb.c)

list(APPEND TEST_EXECUTABLES test_ajsb)

set_target_properties(test_ajsb PROPERTIES
  C_STANDARD 17
  C_STANDARD_REQUIRED YES
)
if("CXX" IN_LIST CMAKE_PROJECT_LANGUAGES)
  set_target_properties(test_ajsb PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
  )
endif()

if(NOT TARGET a_json_schema_builder_library::a_json_schema_builder_library)
  find_package(a_json_schema_builder_library CONFIG REQUIRED)
endif()
target_link_libraries(test_ajsb PRIVATE a_json_schema_builder_library::a_json_schema_builder_library)

if(M_LIB)
  target_link_libraries(test_ajsb PRIVATE ${M_LIB})
endif()

if(MSVC)
  target_compile_options(test_ajsb PRIVATE /W4)
else()
  target_compile_options(test_ajsb PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(A_ENABLE_COVERAGE)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(test_ajsb PRIVATE -O0 -g -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(test_ajsb PRIVATE -fprofile-instr-generate -fcoverage-mapping)
  elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_ajsb PRIVATE -O0 -g --coverage)
    target_link_options(test_ajsb PRIVATE --coverage)
  endif()
endif()

add_test(NAME test_ajsb COMMAND $<TARGET_FILE:test_ajsb>)

enable_testing()

# ---- Coverage aggregation ----
add_custom_target(coverage_report COMMENT "Generate coverage report")

if(A_ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  include(ProcessorCount)
  ProcessorCount(NPROC)
  if(NPROC EQUAL 0)
    set(NPROC 4)
  endif()

  add_custom_command(TARGET coverage_report PRE_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${NPROC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running tests with coverage instrumentation"
  )

  find_program(LLVM_PROFDATA_EXECUTABLE llvm-profdata)
  find_program(LLVM_COV_EXECUTABLE llvm-cov)
  if(LLVM_PROFDATA_EXECUTABLE AND LLVM_COV_EXECUTABLE)
    add_custom_command(TARGET coverage_report POST_BUILD
      COMMAND ${LLVM_PROFDATA_EXECUTABLE} merge -sparse *.profraw -o default.profdata
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Merging coverage data"
    )

    # Normal compiled library path
    add_custom_command(TARGET coverage_report POST_BUILD
      COMMAND ${LLVM_COV_EXECUTABLE} show
              ../liba_json_schema_builder_library_${A_BUILD_VARIANT}.a
              -instr-profile=default.profdata
              -format=html -output-dir=coverage_html
              --ignore-filename-regex='/usr/.*'
              --show-regions --show-line-counts-or-regions
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generated LLVM coverage report → open coverage_html/index.html"
    )
  endif()
endif()
