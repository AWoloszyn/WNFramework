enable_overlay_directory()

add_wn_library(WNFileSystem
  SOURCES
    inc/object_info.h
    inc/WNFactory.h
    inc/WNFile.h
    inc/WNMapping.h
    inc/WNResult.h
    src/WNFactory.cpp
    src/WNMapping.cpp
    src/WNMemoryBackedMapping.cpp
    src/WNMemoryBackedMapping.h
    src/WNSystemPaths.h
    src/WNUtilities.h
  LIBS
    WNUtilities
)

wn_add_subdirectory(test)

post_enable_overlay_directory()