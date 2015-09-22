cmake_minimum_required (VERSION 2.8.7)

############################################################
project (ASDF)
set(ASDF_VERSION 0.0.1)
# The following version should be bumped if the ABI changes. It does
# *not* need to stay in sync with the "display" version above.
set(ASDF_SOVERSION 0.0.1)

include(GNUInstallDirs)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
set(CMAKE_C_FLAGS_RELWITHDEBINFO
    "${CMAKE_C_FLAGS_RELWITHDEBINFO} -Wall -Werror")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Wall -Werror")

############################################################
### Dependencies                                         ###
############################################################

find_package(HDF5 1.8.0 REQUIRED)
include_directories(${HDF5_INCLUDE_DIR})
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
option (TEST "Run unit tests" OFF)

set(Boost_NO_SYSTEM_PATHS FALSE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT /tigress/jas11/packages/boost)
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${BOOST_INCLUDE_DIRS})

############################################################
### Setup directory structure                            ###
############################################################
include_directories (./inc)
include_directories(${BOOST_INCLUDE_DIRS})

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)  # .so
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)  # .a

set (SRC_DIR ${CMAKE_SOURCE_DIR}/src)

############################################################
### ASDF library                                         ###
############################################################
add_library (asdf
             ${SRC_DIR}/ASDF_init.c
             ${SRC_DIR}/ASDF_read.c
             ${SRC_DIR}/ASDF_write.c
             ${SRC_DIR}/parse_sf_parfile.cpp
             ${SRC_DIR}/gen_provenance_id.cpp
             ${SRC_DIR}/gen_provenance_entity.cpp
             ${SRC_DIR}/gen_sf_parfile_provenance.cpp
             ${SRC_DIR}/gen_sf_provenance.cpp
             ${SRC_DIR}/ASDF_provenance.c
             ${SRC_DIR}/ASDF_f.c)

target_link_libraries(asdf
                      ${HDF5_LIBRARIES} ${MPI_C_LIBRARIES})

set_property(TARGET asdf PROPERTY VERSION ${ASDF_VERSION})
set_property(TARGET asdf PROPERTY SOVERSION ${ASDF_SOVERSION})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inc/ASDF_common.h.in
               ${CMAKE_BINARY_DIR}/inc/ASDF_common.h
               @ONLY)

install(TARGETS asdf
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
file(GLOB inc_files inc/*.h)
install(FILES ${inc_files} ${CMAKE_BINARY_DIR}/inc/ASDF_common.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (${CMAKE_VERSION} VERSION_GREATER 3.1)
  set_property(TARGET asdf PROPERTY CXX_STANDARD 11)
else()
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if (COMPILER_SUPPORTS_CXX11)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif (COMPILER_SUPPORTS_CXX0X)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(FATAL_ERROR
            "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. "
            "Please use a different C++ compiler.")
  endif()
endif()

############################################################
### Documentation Generation                             ###
############################################################
# See: https://www.tty1.net/blog/2014/cmake-doxygen_en.html
find_package(Doxygen)
option (BUILD_DOCUMENTATION
        "Create and install the HTML based API documentation (requires Doxygen)"
        ${DOXYGEN_FOUND})

if (BUILD_DOCUMENTATION)
  if (NOT DOXYGEN_FOUND)
    message (FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif ()

  set (doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set (doxyfile ${CMAKE_BINARY_DIR}/Doxyfile)

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  add_custom_target (doc
      command ${DOXYGEN_EXECUTABLE} ${doxyfile}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM)

  install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/html
          DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

install(FILES README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})

############################################################
### Helpers for Downstream Users                         ###
############################################################

# pkg-config
set(PKG_CONFIG_REQUIRES "")
set(PKG_CONFIG_REQUIRES_PRIVATE "")

set(PKG_CONFIG_CFLAGS "-I\${prefix}/\${includedir} ${HDF5_INCLUDE_DIR}")
set(PKG_CONFIG_LIBS "-L\${prefix}/\${libdir} -lasdf")
string(REPLACE ";" " " PKG_CONFIG_LIBS_PRIVATE
       "${HDF5_LIBRARIES}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
)

install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# CMake
include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/asdf"
  PATH_VARS ${PROJECT_NAME}_INCLUDE_DIR ${PROJECT_NAME}_LIBRARY_DIR
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION 0.0.1 #${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/asdf)

############################################################
if (TEST)
  include(CTest)  # enable_testing() is not enough to 'make test'
  add_subdirectory(test)
endif()
add_subdirectory(examples)
