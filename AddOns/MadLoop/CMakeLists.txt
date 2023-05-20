<<<<<<< HEAD
add_definitions(-DMADLOOP_PREFIX="")#FIXME!!
add_library(SherpaML5 SHARED ML5_Interface.C)
install(TARGETS SherpaML5 DESTINATION ${CMAKE_INSTALL_LIBDIR}/SHERPA-MC COMPONENT libs)
set_target_properties(SherpaML5       PROPERTIES POSITION_INDEPENDENT_CODE ON OUTPUT_NAME SherpaML5 SOVERSION 0.0.0)
set_target_properties(SherpaML5       PROPERTIES DEFINE_SYMBOL "")
=======

set(MadLoop_SOURCES ML5_Interface.C)
sherpa_create_git_info(MadLoop)
add_library(SherpaML5 SHARED ${MadLoop_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/Git_Info.C)
sherpa_handle_shared_library(SherpaML5 SherpaML5)
>>>>>>> upstream/master
