cmake_minimum_required(VERSION 3.25)
project(MyCppProject CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt CONFIG REQUIRED)

add_executable(app 
    src/main.cpp
    include/node.hpp
    include/market.hpp
    src/market.cpp
    src/node.cpp
    include/fix_branch_market.hpp
    include/error.hpp
    src/error.cpp
)

target_include_directories(app PRIVATE include)


# Eigen
find_package(Eigen3 REQUIRED NO_MODULE)
target_link_libraries(app PRIVATE Eigen3::Eigen)
