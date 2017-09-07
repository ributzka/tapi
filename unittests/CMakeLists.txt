add_custom_target(TapiUnitTests)
set_target_properties(TapiUnitTests PROPERTIES FOLDER "Tapi Tests")

function(add_tapi_unittest test_dirname)
  add_unittest(TapiUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(FileSystem)
add_subdirectory(libtapi)
add_subdirectory(Path)
add_subdirectory(SDKDB)
