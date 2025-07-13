# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\KingPlayer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\KingPlayer_autogen.dir\\ParseCache.txt"
  "KingPlayer_autogen"
  )
endif()
