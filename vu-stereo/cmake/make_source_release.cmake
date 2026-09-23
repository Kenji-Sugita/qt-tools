# make_source_release.cmake は、配布用のソース zip を作成する CMake スクリプトです。
# CMakeLists.txt の source_release ターゲットから呼び出され、ビルドに必要な
# ソース、画像、翻訳、ドキュメントだけを一時ディレクトリへ集めて zip 化します。
# build/ や既存の配布物は入れず、zip 内のトップディレクトリ名は vu-stereo に固定します。

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

if(NOT DEFINED RELEASE_DIR)
    message(FATAL_ERROR "RELEASE_DIR is required")
endif()

if(NOT DEFINED RELEASE_VERSION)
    message(FATAL_ERROR "RELEASE_VERSION is required")
endif()

set(package_root_name "vu-stereo")
set(stage_dir "${RELEASE_DIR}/_stage")
set(package_dir "${stage_dir}/${package_root_name}")
set(zip_path "${RELEASE_DIR}/vu-stereo-${RELEASE_VERSION}-source.zip")

file(REMOVE_RECURSE "${stage_dir}")
file(MAKE_DIRECTORY "${package_dir}")
file(MAKE_DIRECTORY "${RELEASE_DIR}")

set(source_release_files
    CHANGELOG.md
    CMakeLists.txt
    PRIVACY.md
    README.md
    SIGNING.md
    USER_GUIDE.md
    USER_GUIDE.pdf
    VERSION
    code-signing.txt
)

foreach(file_name IN LISTS source_release_files)
    if(EXISTS "${SOURCE_DIR}/${file_name}")
        file(COPY "${SOURCE_DIR}/${file_name}" DESTINATION "${package_dir}")
    endif()
endforeach()

set(source_release_dirs
    cmake
    images
    include
    src
    translations
)

foreach(dir_name IN LISTS source_release_dirs)
    if(EXISTS "${SOURCE_DIR}/${dir_name}")
        file(COPY "${SOURCE_DIR}/${dir_name}" DESTINATION "${package_dir}")
    endif()
endforeach()

file(REMOVE "${zip_path}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar cf "${zip_path}" --format=zip "${package_root_name}"
    WORKING_DIRECTORY "${stage_dir}"
    RESULT_VARIABLE zip_result
)

if(NOT zip_result EQUAL 0)
    message(FATAL_ERROR "Failed to create ${zip_path}")
endif()

file(REMOVE_RECURSE "${stage_dir}")
message(STATUS "Created ${zip_path}")
