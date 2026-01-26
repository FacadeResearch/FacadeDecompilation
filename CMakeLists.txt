cmake_minimum_required(VERSION 3.10)
project(animEngineDLL)

file(GLOB SOURCES "animEngineDLL/*.cpp")
file(GLOB HEADERS "animEngineDLL/*.h")

include_directories("animEngineDLL")

add_library(animEngineDLL SHARED ${SOURCES} ${HEADERS})

target_link_libraries(animEngineDLL
    opengl32 
    glu32 
    user32 
    gdi32 
    dinput8 
    dsound 
    dxguid
)