include(FetchContent)
FetchContent_Declare(
    vcpkg
    SYSTEM
    GIT_REPOSITORY https://github.com/microsoft/vcpkg/
    GIT_TAG 2026.10.16
)
FetchContent_MakeAvailable(vcpkg)

set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")