set(INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Inputs")
add_definitions(-DINPUT_PATH="${INPUT_PATH}")
add_tapi_unittest(SDKDBTests
  SDKDB_v1.cpp
  )

target_link_libraries(SDKDBTests
  tapiCore
  tapiSDKDB
  )
