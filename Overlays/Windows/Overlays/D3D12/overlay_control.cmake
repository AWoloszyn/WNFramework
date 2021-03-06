include(CheckIncludeFileCXX)

set(ENABLED OFF)

check_include_file_cxx(d3d12.h HAS_D3D12_H)

if (HAS_D3D12_H)
  if(NOT WN_GRAPHICS_DEVICE_TYPES)
    set(ENABLED ON)
  else()
    list(FIND WN_GRAPHICS_DEVICE_TYPES "d3d12" has_d3d12)

    if (has_d3d12 GREATER -1)
      set(ENABLED ON)
    endif()
  endif()
endif()

register_overlay(${ENABLED} D3D12)