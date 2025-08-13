cmake_minimum_required(VERSION 3.10)
project(cpt)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# デバッグビルドの設定
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    add_definitions(-DDEBUG)
endif()

# リリースビルドの設定
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
endif()

# Boostの多倍長整数を使っているので、Boostを探す
find_package(Boost REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

# 全てのcppファイルを集める
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(cpt ${SOURCES})

target_link_libraries(cpt PRIVATE ${Boost_LIBRARIES}) 

# make install で /usr/local/bin にインストール
install(TARGETS cpt DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# デフォルトのインストール先を /usr/local に設定
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "" FORCE)
endif() 