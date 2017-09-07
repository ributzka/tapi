set(SOURCES
  unknown_platform.c
  )

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
set(CMAKE_SHARED_LINKER_FLAGS "-current_version 1 -compatibility_version 1 -Xlinker -no_version_load_command -Xlinker -no_uuid")

add_library(UnknownPlatform
  SHARED
  ${SOURCES}
  )

set_target_properties(UnknownPlatform
  PROPERTIES
  INSTALL_NAME_DIR /usr/lib
  VERSION 1
  SOVERSION 1
  )

install(TARGETS UnknownPlatform
  LIBRARY DESTINATION usr/lib
  )
