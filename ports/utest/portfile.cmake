set(VCPKG_BUILD_TYPE release)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO friendlyanon/utest.h
    REF c24593a29bf3d4f1e79ba1c1be86fa9e9cb538d3
    SHA512 237e7b1c1f9f1222a9622dc88c298ccfa2962df676e4dfe3b4d242a63d64bfa8197c9505df501964c9b1ee6f234d3c3793168b0b699732ab4acc7ddda9d59a0a
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
