target_sources(rnx
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/doris/beacon_from_rinex_line.cpp
    ${CMAKE_SOURCE_DIR}/src/doris/observationtype.cpp
    ${CMAKE_SOURCE_DIR}/src/doris/read_rinex_header.cpp
    ${CMAKE_SOURCE_DIR}/src/doris/doris_rinex.cpp
)
