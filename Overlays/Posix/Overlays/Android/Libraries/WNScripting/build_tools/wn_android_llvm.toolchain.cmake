set(BASE_TOOLCHAIN_FILE $ENV{ANDROID_TOOLCHAIN_FILE} CACHE STRING "The toolchain file to use")

include(${BASE_TOOLCHAIN_FILE})

foreach(flag_var CMAKE_CXX_FLAGS_DEBUG  CMAKE_C_FLAGS_DEBUG)
  string(REGEX REPLACE "-DDEBUG" "-DDEB" ${flag_var} "${${flag_var}}")
endforeach()
add_compile_options("-fvisibility=hidden")
add_compile_options("-Wno-unused-lambda-capture")

set( CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "c++ Debug flags" FORCE)
set( CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "c Debug flags" FORCE)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  # Clang on android compiles with -Oz, this breaks the build in
  # subtle ways.
  add_compile_options("-Os")
endif()

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_LIST_DIR}")
