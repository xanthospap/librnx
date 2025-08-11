cmake_minimum_required(VERSION 3.8)

project(
  rnx
  VERSION 1.1.0
  DESCRIPTION "Space Geodesy data files"
  LANGUAGES CXX
)

# Ensure required libraries are available
find_package(Eigen3   REQUIRED)
find_package(geodesy  REQUIRED)
find_package(datetime REQUIRED)

# Pass the library dependencies to subdirectories
set(PROJECT_DEPENDENCIES Eigen3::Eigen geodesy datetime)

# Define an option for building tests (defaults to ON)
option(BUILD_TESTING "Enable building of tests" ON)

# compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
add_compile_options(-Wall 
  -Wextra 
  -Werror 
  -pedantic 
  -W 
  -Wshadow 
  $<$<CONFIG:Release>:-O2>
  $<$<CONFIG:Release>:-march=native>
  $<$<CONFIG:Debug>:-g>
  $<$<CONFIG:Debug>:-pg> 
  $<$<CONFIG:Debug>:-Wdisabled-optimization>
  )
add_compile_definitions(
  $<$<CONFIG:Debug>:DEBUG>
)

# the library and includes
add_library(rnx)
target_include_directories(rnx
PUBLIC
  # Public headers during build time
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  # Public headers during install time
  $<INSTALL_INTERFACE:include/rnx>
PRIVATE
  # Private/internal headers only during build time (src/core)
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  # Internal/private headers during install time (from include/rnx/core)
  $<INSTALL_INTERFACE:include/rnx/core>
)

# library source code
add_subdirectory(src/doris)

# The tests
#if(BUILD_TESTING)
#  include(CTest)
#  enable_testing()
#endif()

# install library
install(TARGETS rnx
        EXPORT rnxTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

# Install public headers at: $PREFIX/rnx/...
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
	DESTINATION include/rnx
)

install(EXPORT rnxTargets
        FILE rnxTargets.cmake
        NAMESPACE dso::
        DESTINATION lib/cmake
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "rnxConfigVersion.cmake"
  VERSION ${rnx_version}
  COMPATIBILITY AnyNewerVersion
)

install(FILES "rnxConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/rnxConfigVersion.cmake"
  DESTINATION lib/cmake
)
