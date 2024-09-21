cmake_minimum_required(VERSION 3.12)

project(ImgMix)

# add_compile_options(-mwindows)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    C:/opencv/build/x64/mingw/install/include
    C:/opencv/build/x64/mingw/install/include/opencv2
)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/opencv/copy_modify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/opencv/tank.cpp
)

add_link_options(-Wl,-subsystem,windows,-entry,mainCRTStartup)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(ImageMix ${SRC})

target_link_libraries(ImageMix PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bin/libopencv_world490.dll)
