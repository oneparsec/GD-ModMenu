cmake_minimum_required(VERSION 3.0)

add_library(imgui_hook STATIC
    imgui_hook.cpp
    imgui_hook.h
    imgui/imgui.cpp
	imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_opengl2.cpp
    imgui/imgui_impl_opengl2.h
    imgui/imgui_impl_win32.cpp
    imgui/imgui_impl_win32.h
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    kiero/kiero.cpp
    kiero/kiero.h
)
set_property(TARGET imgui_hook PROPERTY CXX_STANDARD 17)
target_include_directories(imgui_hook PRIVATE imgui
    ../cocos-headers/cocos2dx/
    ../cocos-headers/cocos2dx/include
    ../cocos-headers/cocos2dx/kazmath/include
    ../cocos-headers/cocos2dx/platform/win32
    ../cocos-headers/cocos2dx/platform/third_party/win32
    ../cocos-headers/cocos2dx/platform/third_party/win32/OGLES
)