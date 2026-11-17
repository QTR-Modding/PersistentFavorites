# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QTR-Modding/CLibUtilsQTR
    REF 57710e866eb3238134ff9aa673b5dc2fb4fd0fb4
    SHA512 9cb2475d2f98dd931224c9a5bc28643c5118883d68c71b949169ee30985114e9035d447d46237437b72abdcd293326d2bdca9214eae2a695410fbdaa52fedd8c
    HEAD_REF main
)

# Install codes
set(CLibUtilsQTR_SOURCE	${SOURCE_PATH}/include/CLibUtilsQTR)
file(INSTALL ${CLibUtilsQTR_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")