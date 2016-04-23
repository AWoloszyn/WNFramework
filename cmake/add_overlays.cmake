set(WN_DISABLED_OVERLAYS "")
set(WN_DISABLED_OVERLAY_NAMES "")
set(WN_ENABLED_OVERLAYS "")
set(WN_ENABLED_OVERLAY_NAMES "")

function(subdirectories directory result_list)
  set(all_directories)
  FILE(GLOB children RELATIVE ${directory} ${directory}/*)

  FOREACH(child ${children})
    IF(IS_DIRECTORY ${directory}/${child})
        LIST(APPEND all_directories "${directory}/${child}")
    ENDIF()
  ENDFOREACH()

  set(${result_list} ${all_directories} PARENT_SCOPE)
endfunction()

macro(append_to_overlay_var var val)
  set(local ${${var}})
  list(APPEND local ${val})
  set(${var} ${local})
  set(${var} ${local} PARENT_SCOPE)
endmacro()

macro(set_overlay_var var val)
  set(${var} ${val} PARENT_SCOPE)
  set(${var} ${val})
endmacro()

macro(register_all_overlays)
  set(WN_FORCE_DISABLED_OVERLAY OFF)
  set(OVERLAY_NAME_STACK)
  recursive_register_overlays(${CMAKE_CURRENT_SOURCE_DIR} OVERLAY_NAME_STACK)
endmacro()

macro(propagate_to_parent variable)
  set(TEMP ${${variable}})
  set(${variable} ${TEMP} PARENT_SCOPE)
endmacro()

macro(propagate_from_parent variable)
  set(${variable} ${${variable}})
endmacro()

macro(merge_paths out_path)
  set(add_slash OFF)
  set(${out_path})
  foreach(SEP ${ARGN})
    if (add_slash)
      set(${out_path} "${${out_path}}/")
    endif()
    set(${out_path} "${${out_path}}${SEP}")
    set(add_slash ON)
  endforeach()
endmacro()

function(recursive_register_overlays root_dir overlay_stack)
  set(current_overlay_name ${WN_CURRENT_OVERLAY_NAME})
  set(force_disabled ${WN_FORCE_DISABLED_OVERLAY})
  propagate_from_parent(WN_ENABLED_OVERLAYS)
  propagate_from_parent(WN_ENABLED_OVERLAY_NAMES)
  propagate_from_parent(WN_DISABLED_OVERLAYS)
  propagate_from_parent(WN_DISABLED_OVERLAY_NAMES)

  subdirectories(${root_dir}/Overlays overlay_dirs)
  FOREACH(overlay ${overlay_dirs})
    set(WN_FORCE_DISABLED_OVERLAY ${force_disabled})
    set(was_enabled OFF)
    if (EXISTS ${overlay}/overlay_control.cmake)
      set(WN_CURRENT_OVERLAY_DIR ${overlay})
      include(${overlay}/overlay_control.cmake)
      if (${was_enabled})
        set(WN_FORCE_DISABLED_OVERLAY OFF)
      else()
        set(WN_FORCE_DISABLED_OVERLAY ON)
      endif()
      recursive_register_overlays(${overlay} ${overlay_stack})
      set(WN_FORCE_DISABLED_OVERLAY ${force_disabled})
      set(WN_CURRENT_OVERLAY_NAME ${current_overlay_name})
    endif()
  ENDFOREACH()
  propagate_to_parent(WN_ENABLED_OVERLAYS)
  propagate_to_parent(WN_ENABLED_OVERLAY_NAMES)
  propagate_to_parent(WN_DISABLED_OVERLAYS)
  propagate_to_parent(WN_DISABLED_OVERLAY_NAMES)
endfunction()

macro(register_overlay enabled name)
  if (WN_FORCE_DISABLED_OVERLAY OR NOT ${enabled})
    register_disabled_overlay(${name})
    set(was_enabled OFF)
  else()
    register_enabled_overlay(${name})
    set(was_enabled ON)
  endif()
endmacro()

macro(register_enabled_overlay name)
  set(enabled_overlays ${WN_ENABLED_OVERLAYS})
  set(enabled_overlay_names ${WN_ENABLED_OVERLAY_NAMES})

  merge_paths(WN_CURRENT_OVERLAY_NAME ${WN_CURRENT_OVERLAY_NAME} ${name})
  list(APPEND enabled_overlays ${WN_CURRENT_OVERLAY_DIR})
  list(APPEND enabled_overlay_names ${WN_CURRENT_OVERLAY_NAME})
  set(WN_ENABLED_OVERLAYS ${enabled_overlays})
  set(WN_ENABLED_OVERLAY_NAMES ${enabled_overlay_names})
endmacro()

macro(register_disabled_overlay name)
  set(disabled_overlays ${WN_DISABLED_OVERLAYS})
  set(disabled_overlay_names ${WN_DISABLED_OVERLAY_NAMES})
  merge_paths(WN_CURRENT_OVERLAY_NAME ${WN_CURRENT_OVERLAY_NAME} ${name})

  list(APPEND disabled_overlays ${CMAKE_CURRENT_SOURCE_DIR})
  list(APPEND disabled_overlay_names ${WN_CURRENT_OVERLAY_NAME})

  set(WN_DISABLED_OVERLAYS ${disabled_overlays})
  set(WN_DISABLED_OVERLAY_NAMES ${disabled_overlay_names})
endmacro()

macro(enable_overlay_directory)
  file(RELATIVE_PATH relative_root ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})

  foreach(enabled_overlay RANGE 0 ${WN_MAX_ENABLED_OVERLAY})
    list(GET WN_ENABLED_OVERLAYS ${enabled_overlay} overlay_root)
    list(GET WN_ENABLED_OVERLAY_NAMES ${enabled_overlay} overlay_name)
    set(WN_CURRENT_OVERLAY ${overlay_name})
    set(WN_CURRENT_OVERLAY_ROOT ${overlay_root})
    if (IS_DIRECTORY ${overlay_root}/${relative_root})
      add_subdirectory(${overlay_root}/${relative_root} ${overlay_name})
    endif()
  endforeach()
endmacro()

macro(enable_overlay_file)
  file(RELATIVE_PATH relative_file ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_FILE})
 
  foreach(enabled_overlay RANGE 0 ${WN_MAX_ENABLED_OVERLAY})
    list(GET WN_ENABLED_OVERLAYS ${enabled_overlay} overlay_root)
    list(GET WN_ENABLED_OVERLAY_NAMES ${enabled_overlay} overlay_name)
    set(WN_CURRENT_OVERLAY ${overlay_name})
    set(WN_CURRENT_OVERLAY_ROOT ${overlay_root})
    if (EXISTS ${overlay_root}/${relative_file})
      include(${overlay_root}/${relative_file})
    endif()
  endforeach()
 endmacro()

macro(overlay_named_file name)
  foreach(enabled_overlay RANGE 0 ${WN_MAX_ENABLED_OVERLAY})
    list(GET WN_ENABLED_OVERLAYS ${enabled_overlay} overlay_root)
    list(GET WN_ENABLED_OVERLAY_NAMES ${enabled_overlay} overlay_name)
    set(WN_CURRENT_OVERLAY ${overlay_name})
    set(WN_CURRENT_OVERLAY_ROOT ${overlay_root})
    if (EXISTS ${overlay_root}/${name})
      include(${overlay_root}/${name})
    endif()
  endforeach()
endmacro()

# Register all valid overlays
register_all_overlays()

list(LENGTH WN_ENABLED_OVERLAYS WN_NUM_ENABLED_OVERLAYS)
list(LENGTH WN_DISABLED_OVERLAYS WN_NUM_DISABLED_OVERLAYS)

set(WN_ALL_OVERLAYS
  "${WN_ENABLED_OVERLAYS};${WN_DISABLED_OVERLAYS}")
set(WN_ALL_OVERLAY_NAMES
  "${WN_ENABLED_OVERLAY_NAMES};${WN_DISABLED_OVERLAY_NAMES}")

math(EXPR WN_MAX_ENABLED_OVERLAY "${WN_NUM_ENABLED_OVERLAYS} - 1")
math(EXPR WN_MAX_DISABLED_OVERLAY "${WN_NUM_DISABLED_OVERLAYS} - 1")
message(STATUS "[${WN_NUM_ENABLED_OVERLAYS}] Enabled Overlays: ${WN_ENABLED_OVERLAY_NAMES}")
message(STATUS "[${WN_NUM_DISABLED_OVERLAYS}] Disabled Overlays: ${WN_DISABLED_OVERLAY_NAMES}")

foreach(enabled_overlay RANGE 0 ${WN_MAX_ENABLED_OVERLAY})
  list(GET WN_ENABLED_OVERLAYS ${enabled_overlay} overlay_root)
  list(GET WN_ENABLED_OVERLAY_NAMES ${enabled_overlay} overlay_name)
  string(REPLACE "/" "_" SANITIZED_OVERLAY_NAME ${overlay_name})
  set(${SANITIZED_OVERLAY_NAME}_ROOT_DIR ${overlay_root})
endforeach()

