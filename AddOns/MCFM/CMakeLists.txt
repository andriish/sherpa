<<<<<<< HEAD
add_library(SherpaMCFM SHARED MCFM_Interface.C)
install(TARGETS SherpaMCFM DESTINATION ${CMAKE_INSTALL_LIBDIR}/SHERPA-MC COMPONENT libs)
set_target_properties(SherpaMCFM       PROPERTIES POSITION_INDEPENDENT_CODE ON OUTPUT_NAME SherpaMCFM SOVERSION 0.0.0)
target_include_directories(SherpaMCFM  PRIVATE ${MCFM_INCLUDE_DIR})
target_link_libraries(SherpaMCFM  PRIVATE ${MCFM_LIBRARY})
set_target_properties(SherpaMCFM       PROPERTIES DEFINE_SYMBOL "")
target_compile_options(SherpaMCFM       PRIVATE -fopenmp)
#-fopenmp ?
=======

set(MCFM_SOURCES MCFM_Interface.C)
sherpa_create_git_info(MCFM)

add_library(SherpaMCFM SHARED ${MCFM_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/Git_Info.C)
sherpa_handle_shared_library(SherpaMCFM SherpaMCFM)
target_include_directories(SherpaMCFM  PRIVATE ${MCFM_INCLUDE_DIR})
target_link_libraries(SherpaMCFM  PRIVATE ${MCFM_LIBRARY})

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  target_compile_options(SherpaMCFM       PRIVATE -fopenmp)
endif()
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
  target_compile_options(SherpaMCFM       PRIVATE -qopenmp)
endif()
>>>>>>> upstream/master
