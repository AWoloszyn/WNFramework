include (${WNFramework_SOURCE_DIR}/cmake/make_preprocessor_symbol.cmake)
include (CheckSymbolExists)

function(wn_check_any_symbol_exists LIST_VARIABLE_NAME VARIABLE)
  foreach(SYMBOL ${${LIST_VARIABLE_NAME}})
    set(CHECK_VARIABLE_NAME HAS_${SYMBOL})
    string(TOUPPER ${CHECK_VARIABLE_NAME} CHECK_VARIABLE_NAME)
    set(CMAKE_PASSTHROUGH_FLAGS "")

#    if (DEFINED CMAKE_ANDROID_ARCH)
#      list(APPEND CMAKE_PASSTHROUGH_FLAGS
#        "-DCMAKE_ANDROID_ARCH=${CMAKE_ANDROID_ARCH}")
#    endif()
#    if (DEFINED ANDROID_NATIVE_API_LEVEL)
#      list(APPEND CMAKE_PASSTHROUGH_FLAGS
#          "-DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}")
#    endif()
#    if (DEFINED "CMAKE_ANDROID_API_MIN")
#      list(APPEND CMAKE_PASSTHROUGH_FLAGS
#          "-DCMAKE_ANDROID_API_MIN=${CMAKE_ANDROID_API_MIN}")
#    endif()
#
    check_symbol_exists(${SYMBOL} "" ${CHECK_VARIABLE_NAME})
    if (${CHECK_VARIABLE_NAME})
      set(${VARIABLE} true PARENT_SCOPE)
      break()
    else()
      set(${VARIABLE} false PARENT_SCOPE)
    endif()
  endforeach()
endfunction()


# list of all suported architectures
set(ARCHITECTURE_LIST "x86-64" x86 ARM64 ARM)

# architectural preprocessor symbols x86-64
set(X86_64_SYMBOLS
  __amd64__
  __amd64
  __x86_64__
  __x86_64
  _M_X64
  _M_AMD64
)

# architectural preprocessor symbols x86
set(X86_SYMBOLS
  i386
  __i386
  __i386__
  __IA32__
  _M_I86
  _M_IX86
  __X86__
  _X86_
  __THW_INTEL__
  __I86__
  __INTEL__
  __386
)

# architectural preprocessor symbols ARM64
set(ARM64_SYMBOLS
  __aarch64__
)

# architectural preprocessor symbols ARM
set(ARM_SYMBOLS
  __arm__
  __arm
  __thumb__
  __TARGET_ARCH_ARM
  __TARGET_ARCH_THUMB
  _ARM
  _M_ARM
  _M_ARMT
)

# check for architectures using preprocessor symbols. Symbol list names must
# match the format [architecture, all caps, alphanumeric and _ only]_SYMBOLS.
foreach(ARCHITECTURE ${ARCHITECTURE_LIST})
  wn_make_preprocessor_symbol(${ARCHITECTURE} SANITIZE_ARCHITECTURE)

  set(ARCHITECTURE_SYMBOL_LIST_NAME ${SANITIZE_ARCHITECTURE}_SYMBOLS)
  set(ARCHITECTURE_HAS_VARIABLE_NAME HAS_${SANITIZE_ARCHITECTURE}_SYMBOL)

  wn_check_any_symbol_exists(
    ${ARCHITECTURE_SYMBOL_LIST_NAME}
    ${ARCHITECTURE_HAS_VARIABLE_NAME}
  )

  if (${ARCHITECTURE_HAS_VARIABLE_NAME})
    set(WN_ARCHITECTURE ${ARCHITECTURE})

    break()
  endif()
endforeach()

if (NOT DEFINED WN_ARCHITECTURE)
  message(FATAL_ERROR "Failed to determine architecture")
endif()

message(STATUS "Architecture is ${WN_ARCHITECTURE}")