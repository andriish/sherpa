set(Beam_Remnants_SOURCES Continued_PDF.C
                          Hadron_Dissociation.C
                          Remnant_Handler.C
                          Colour_Generator.C)

set(Beam_Remnants_HEADERS Continued_PDF.H
                          Hadron_Dissociation.H
                          Remnant_Handler.H
                          Colour_Generator.H)

sherpa_create_git_info(Beam_Remnants)
 
add_library(ShrimpsBeamRemnants SHARED ${Beam_Remnants_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/Git_Info.C )
sherpa_handle_shared_library(ShrimpsBeamRemnants ShrimpsBeamRemnants)
