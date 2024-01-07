cmake_minimum_required(VERSION 3.7.0)
project(Gui VERSION 1.0.0 LANGUAGES CXX)

# Add gui.cpp file of the project root directory as a source file
set(SOURCE_DIR ".")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add library target with source files listed in SOURCE_FILES variable
add_library(Gui "gui.cpp" "draw.cpp" "tables.cpp" "widgets.cpp" "backends/opengl3.cpp" "backends/sdl2.cpp")
set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED OpenGL GLX EGL)
include_directories(${SDL2_INCLUDE_DIRS} ${libGLX_mesa_INCLUDE_DIRS} "." "backends")
target_link_libraries(Gui ${SDL2_LIBRARIES} ${libGLX_mesa_LIBRARIES})
