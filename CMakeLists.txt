cmake_minimum_required(VERSION 3.30)
project(cpt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

# デバッグビルドの設定
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    add_definitions(-DDEBUG)
endif()

# リリースビルドの設定
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
endif()

# Boostの多倍長整数と数学定数を使う。
# まずシステムにインストール済みのBoostを探し、見つからない場合は
# CPT_FETCH_BOOST が有効なら、ヘッダーを含むBoostのソースアーカイブを取得する。
option(CPT_FETCH_BOOST
    "Download Boost automatically when it is not installed"
    ON
)

set(CPT_BOOST_VERSION "" CACHE STRING
    "Boost version to fetch; empty means the latest stable release"
)

find_package(Boost QUIET)
if(NOT Boost_FOUND)
    if(NOT CPT_FETCH_BOOST)
        message(FATAL_ERROR
            "Boost was not found. Install Boost or configure with "
            "-DCPT_FETCH_BOOST=ON."
        )
    endif()

    message(STATUS "Boost not found. Downloading Boost headers...")
    include(FetchContent)

    if(CPT_BOOST_VERSION STREQUAL "")
        set(_boost_release_json "${CMAKE_BINARY_DIR}/boost-latest-release.json")
        file(DOWNLOAD
            "https://api.github.com/repos/boostorg/boost/releases/latest"
            "${_boost_release_json}"
            STATUS _boost_download_status
            TIMEOUT 30
            TLS_VERIFY ON
            HTTPHEADER
                "Accept: application/vnd.github+json"
                "User-Agent: cpt-cmake"
        )

        list(GET _boost_download_status 0 _boost_download_code)
        list(GET _boost_download_status 1 _boost_download_message)
        if(NOT _boost_download_code EQUAL 0)
            message(FATAL_ERROR
                "Failed to query the latest Boost release: ${_boost_download_message}"
            )
        endif()

        file(READ "${_boost_release_json}" _boost_release_json_text)
        string(JSON _boost_release_tag
            ERROR_VARIABLE _boost_json_error
            GET "${_boost_release_json_text}" tag_name
        )
        if(_boost_json_error)
            message(FATAL_ERROR
                "Failed to read the latest Boost release metadata: ${_boost_json_error}"
            )
        endif()

        # GitHubのタグから安定版番号だけを取り出す。
        if(NOT _boost_release_tag MATCHES "^boost-[0-9]+\\.[0-9]+\\.[0-9]+(-[0-9]+)?$")
            message(FATAL_ERROR
                "GitHub returned a non-stable Boost release tag: ${_boost_release_tag}"
            )
        endif()
        string(REGEX REPLACE
            "^boost-([0-9]+\\.[0-9]+\\.[0-9]+)(-[0-9]+)?$"
            "\\1"
            CPT_BOOST_VERSION
            "${_boost_release_tag}"
        )
    endif()

    if(NOT CPT_BOOST_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
        message(FATAL_ERROR
            "CPT_BOOST_VERSION must be a semantic version such as 1.92.0"
        )
    endif()

    string(REPLACE "." "_" _boost_version_underscore "${CPT_BOOST_VERSION}")
    set(_boost_content_name "boost_headers_${_boost_version_underscore}")
    set(_boost_archive_url
        "https://archives.boost.org/release/${CPT_BOOST_VERSION}/source/boost_${_boost_version_underscore}.tar.gz"
    )

    message(STATUS "Using Boost ${CPT_BOOST_VERSION}")
    FetchContent_Declare(
        ${_boost_content_name}
        URL "${_boost_archive_url}"
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
    )
    FetchContent_MakeAvailable(${_boost_content_name})
    FetchContent_GetProperties(
        ${_boost_content_name}
        SOURCE_DIR _boost_source_dir
    )

    # Boost.Multiprecision と Boost.Math はヘッダーオンリーなので、
    # 取得したソースディレクトリをインクルードパスとして使う。
    set(Boost_INCLUDE_DIRS "${_boost_source_dir}")
    set(Boost_LIBRARIES "")
endif()

find_package(jpp QUIET)
if(NOT jpp_FOUND)
    message(STATUS "jpp not found. Downloading and building jpp via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        jpp
        GIT_REPOSITORY https://github.com/clever-elsie/jpp.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(jpp)
endif()


include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

# 全てのcppファイルを集める
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(cpt ${SOURCES})

target_link_libraries(cpt PRIVATE ${Boost_LIBRARIES} jpp::jpp) 

# DESTINATION はインストールプレフィックスからの相対パスにする。
# これにより -DCMAKE_INSTALL_PREFIX=... と
# cmake --install ... --prefix ... のどちらでもインストール先を変更できる。
install(TARGETS cpt
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
