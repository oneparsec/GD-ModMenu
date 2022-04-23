cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ModMenu)

add_library(ModMenu SHARED src/dllmain.cpp)

target_include_directories(ModMenu PRIVATE
  libraries/imgui
  libraries/imgui/imgui
  libraries/minhook/include
  libraries/cocos-headers/cocos2dx/
  libraries/cocos-headers/cocos2dx/include
  libraries/cocos-headers/cocos2dx/kazmath/include
  libraries/cocos-headers/cocos2dx/platform/win32
  libraries/cocos-headers/cocos2dx/platform/third_party/win32
  libraries/cocos-headers/cocos2dx/platform/third_party/win32/OGLES
  libraries/imgui/kiero
  libraries/imgui/kiero/minhook
  libraries/imgui/kiero/minhook/src
  libraries/imgui/kiero/minhook/include
  libraries/gd.h
  libraries/gd.h/include
  src/
)

add_subdirectory(libraries/minhook)
add_subdirectory(libraries/imgui)
add_subdirectory(libraries/cocos-headers)
target_link_libraries(ModMenu
  ${CMAKE_SOURCE_DIR}/libraries/cocos-headers/cocos2dx/libcocos2d.lib
  ${CMAKE_SOURCE_DIR}/libraries/glew32.lib
  imgui_hook
  minhook
  opengl32
  cocos2d
)