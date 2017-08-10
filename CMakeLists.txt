
# Requires "FindSDL2.cmake" and "FindSDL2_image.cmake" in ./cmake/
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "./cmake")

cmake_minimum_required(VERSION 3.6)
project(AsciiMap)

set(CMAKE_CXX_STANDARD 14)

FILE(GLOB SOURCE_FILES src/*.cpp)

add_executable( AsciiMap ${SOURCE_FILES} )


find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(${SDL2_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR})
target_link_libraries(AsciiMap ${SDL2_LIBRARY}
                                ${SDL2_IMAGE_LIBRARIES})