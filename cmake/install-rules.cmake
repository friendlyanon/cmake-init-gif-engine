if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/gif_engine CACHE PATH "")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package gif_engine)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT gif_engine_Development
)

install(
    TARGETS gif_engine_gif_engine
    EXPORT gif_engineTargets
    RUNTIME #
    COMPONENT gif_engine_Runtime
    LIBRARY #
    COMPONENT gif_engine_Runtime
    NAMELINK_COMPONENT gif_engine_Development
    ARCHIVE #
    COMPONENT gif_engine_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    gif_engine_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(gif_engine_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${gif_engine_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT gif_engine_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${gif_engine_INSTALL_CMAKEDIR}"
    COMPONENT gif_engine_Development
)

install(
    EXPORT gif_engineTargets
    NAMESPACE gif_engine::
    DESTINATION "${gif_engine_INSTALL_CMAKEDIR}"
    COMPONENT gif_engine_Development
)

# Make the build directory importable for the fuzzer project
configure_file(cmake/install-config.cmake "${package}Config.cmake" COPYONLY)
export(EXPORT gif_engineTargets NAMESPACE gif_engine::)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
