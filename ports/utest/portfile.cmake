set(VCPKG_BUILD_TYPE release)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO friendlyanon/utest.h
    REF 326c9c1d13536787b76a9753066773fe53326ee2
    SHA512 3cfd824a52cf716f45229354f2eea138c27098a115c3a809990272dccec6616de0ed5101b5c4583369fa70523ba0c663d74efb88ff4c6a06ebcf4bd7d2421e0e
    HEAD_REF master
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_cmake_install()

file(
    INSTALL "${SOURCE_PATH}/LICENSE"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/utest"
    RENAME copyright
    MESSAGE_NEVER
)
