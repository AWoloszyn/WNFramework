#This is a slight modification to the GetHostTriple found in the LLVM
#source tree. The License is copied here in full.

#==============================================================================
#LLVM Release License
#==============================================================================
#University of Illinois/NCSA
#Open Source License
#
#Copyright (c) 2003-2015 University of Illinois at Urbana-Champaign.
#All rights reserved.
#
#Developed by:
#
#    LLVM Team
#
#    University of Illinois at Urbana-Champaign
#
#    http://llvm.org
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of
#this software and associated documentation files (the "Software"), to deal with
#the Software without restriction, including without limitation the rights to
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
#of the Software, and to permit persons to whom the Software is furnished to do
#so, subject to the following conditions:
#
#    * Redistributions of source code must retain the above copyright notice,
#      this list of conditions and the following disclaimers.
#
#    * Redistributions in binary form must reproduce the above copyright notice,
#      this list of conditions and the following disclaimers in the
#      documentation and/or other materials provided with the distribution.
#
#    * Neither the names of the LLVM Team, University of Illinois at
#      Urbana-Champaign, nor the names of its contributors may be used to
#      endorse or promote products derived from this Software without specific
#      prior written permission.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
#FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
#CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
#SOFTWARE.

# Returns the host triple.
# Invokes config.guess

function( get_host_triple var )
  if ( WN_IS_TEGRA )
    set ( value "${WN_TEGRA_HOST}")
  elseif( MSVC )
    if( CMAKE_CL_64 )
      set( value "x86_64-pc-win32" )
    else()
      set( value "i686-pc-win32" )
    endif()
  elseif( MINGW AND NOT MSYS )
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set( value "x86_64-w64-mingw32" )
    else()
      set( value "i686-pc-mingw32" )
    endif()
  elseif ( ANDROID )
    set( value ${ANDROID_LLVM_TRIPLE})
  else( MSVC )
    set(config_guess ${LLVM_MAIN_SRC_DIR}/autoconf/config.guess)
    execute_process(COMMAND sh ${config_guess}
      RESULT_VARIABLE TT_RV
      OUTPUT_VARIABLE TT_OUT
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if( NOT TT_RV EQUAL 0 )
      message(FATAL_ERROR "Failed to execute ${config_guess}")
    endif( NOT TT_RV EQUAL 0 )
    set( value ${TT_OUT} )
  endif( WN_IS_TEGRA )
  set( ${var} ${value} PARENT_SCOPE )
  message(STATUS "Target triple: ${value}")
endfunction( get_host_triple var )
