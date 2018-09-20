cmake_minimum_required(VERSION 3.6)

if (NOT DEFINED ENV{TRAVIS_BUILD_NUMBER})
    set(ENV{TRAVIS_BUILD_NUMBER} 1)
endif ()

add_subdirectory(${PROJECT_SOURCE_DIR}/src)

enable_testing()