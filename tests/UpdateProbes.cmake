# Current 014/612 support link/menu probes. Root owns execution and SHA binding.
if(TARGET HonestMix)
 set(HM_UPDATE_PRODUCT_TARGET HonestMix)
 set(HM_UPDATE_CONNECT_DEFINE 1)
elseif(TARGET HonestMixHeadphone)
 set(HM_UPDATE_PRODUCT_TARGET HonestMixHeadphone)
 set(HM_UPDATE_CONNECT_DEFINE 0)
else()
 message(FATAL_ERROR "Support probes require the actual product target")
endif()
juce_add_console_app(UpdateLinkTests PRODUCT_NAME "HONESTMIX Support Link Tests")
target_sources(UpdateLinkTests PRIVATE tests/UpdateLinkTests.cpp)
target_link_libraries(UpdateLinkTests PRIVATE ${HM_UPDATE_PRODUCT_TARGET})
juce_add_console_app(UpdateEditorProbe PRODUCT_NAME "HONESTMIX Support Editor Probe")
target_sources(UpdateEditorProbe PRIVATE tests/UpdateEditorProbe.cpp)
target_compile_definitions(UpdateEditorProbe PRIVATE HM_UPDATE_PROBE_CONNECT=${HM_UPDATE_CONNECT_DEFINE})
target_link_libraries(UpdateEditorProbe PRIVATE ${HM_UPDATE_PRODUCT_TARGET})
if(WIN32)
 foreach(hm_update_probe UpdateLinkTests UpdateEditorProbe)
  target_compile_options(${hm_update_probe} PRIVATE /utf-8)
  set(hm_update_resource_dir "${CMAKE_CURRENT_BINARY_DIR}/${hm_update_probe}_artefacts/JuceLibraryCode")
  add_custom_command(OUTPUT "${hm_update_resource_dir}/${hm_update_probe}_resources.rc"
   APPEND DEPENDS "${hm_update_resource_dir}/Info.txt")
 endforeach()
elseif(APPLE)
 if(NOT DEFINED HM_CRYPTO_DYLIB)
  message(FATAL_ERROR "Mac support probes require the actual existing CryptoKit bridge target")
 endif()
 foreach(hm_update_probe UpdateLinkTests UpdateEditorProbe)
  set_target_properties(${hm_update_probe} PROPERTIES BUILD_RPATH "@loader_path/runtime"
   INSTALL_RPATH "@loader_path/runtime" BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH_USE_LINK_PATH FALSE
   C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden OBJCXX_VISIBILITY_PRESET hidden
   VISIBILITY_INLINES_HIDDEN TRUE)
  add_custom_command(TARGET ${hm_update_probe} POST_BUILD
   COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${hm_update_probe}>/runtime"
   COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${HM_CRYPTO_DYLIB}"
    "$<TARGET_FILE_DIR:${hm_update_probe}>/runtime/libHonestMixMacCrypto.dylib" VERBATIM)
 endforeach()
endif()
