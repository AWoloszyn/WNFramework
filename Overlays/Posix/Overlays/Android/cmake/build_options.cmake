set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}")
 
# -DDEBUG conflicts with some libraries, remove it here
string(REPLACE "-DDEBUG" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

add_compile_options(-D_WN_ANDROID)
add_compile_options(-D_XOPEN_SOURCE=600)

if (NOT ANDROID_SDK)
  message(STATUS "Warning ANDROID_SDK not defined, tests will not work")
else()
  string(REPLACE "\\" "/" ANDROID_SDK ${ANDROID_SDK})
endif()

find_host_program(WN_PYTHON Python)