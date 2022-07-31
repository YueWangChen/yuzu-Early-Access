# Header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO tlk00/BitMagic
    REF v7.11.2
    SHA512 182456fc351a7df0cba0bb5b35d91bb8055891bf91a09fc5394149f3b0b1bd85b63ce88497d62ed68bb892481c15264cf44059d43023d1f2155ed43cafe3b933
    HEAD_REF master

)

file(GLOB HEADER_LIST "${SOURCE_PATH}/src/*.h")
file(INSTALL ${HEADER_LIST} DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)