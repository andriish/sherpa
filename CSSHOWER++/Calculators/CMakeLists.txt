set(Calculators_SOURCES CF_QCD.C
                        CF_QED.C
                        CF_EW_FFV.C
                        CF_Triangle.C
                        CF_HV.C
                        LF_SSV.C
                        LF_FFV.C
                        LF_VVV.C
                        LF_Triangle.C)

sherpa_create_git_info(Calculators)
 
add_library(CSCalculators SHARED ${Calculators_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/Git_Info.C )
sherpa_handle_shared_library(CSCalculators CSCalculators)
