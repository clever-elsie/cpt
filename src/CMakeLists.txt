cmake_minimum_required(VERSION 3.10)
project(cpt)

set(CMAKE_CXX_STANDARD 20)

# Boostの多倍長整数を使っているので、Boostを探す
find_package(Boost REQUIRED)

# サブディレクトリのヘッダも含める
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/calc
    ${CMAKE_SOURCE_DIR}/src/calc/expr
    ${CMAKE_SOURCE_DIR}/src/calc/expr/node
    ${CMAKE_SOURCE_DIR}/src/calc/expr/node/reserved
)

# 全てのcppファイルを集める
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(cpt ${SOURCES})

target_link_libraries(cpt PRIVATE ${Boost_LIBRARIES}) 