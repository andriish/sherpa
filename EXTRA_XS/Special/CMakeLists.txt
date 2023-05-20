<<<<<<< HEAD

########################################################################
#  Automatically or semiautomaticaly generated, do not edit.
########################################################################
# The following input was used
# Makefile.am

########################################################################
#Special/Makefile.am
 
 #pkglib_LTLIBRARIES = libExtraXSSpecial.la)
 set(GITTAG  EXTRA_XS/Special)
 #include ../../git.make)
set(Special_SOURCES  Instanton.C)
creategitinfo(Special ../Special/)
list(TRANSFORM Special_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/../Special/")
list(APPEND Special_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Git_Info.C)
 
add_library(ExtraXSSpecial SHARED 
                             ${Special_SOURCES} 
)
target_include_directories(ExtraXSSpecial PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS ExtraXSSpecial DESTINATION ${CMAKE_INSTALL_LIBDIR}/SHERPA-MC COMPONENT libs)
set_target_properties(ExtraXSSpecial        PROPERTIES POSITION_INDEPENDENT_CODE ON OUTPUT_NAME ExtraXSSpecial SOVERSION 0.0.0)
set_target_properties(ExtraXSSpecial        PROPERTIES DEFINE_SYMBOL "")
=======
set(Special_SOURCES Instanton.C)

sherpa_create_git_info(Special)
 
add_library(ExtraXSSpecial SHARED ${Special_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/Git_Info.C )
sherpa_handle_shared_library(ExtraXSSpecial ExtraXSSpecial)
>>>>>>> upstream/master
