cmake_minimum_required(VERSION 3.6)

project(Example)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${CMAKE_PROJECT_NAME} SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../gui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/android.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/opengl3.cpp
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
  OPENGL_ES3
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../backends
  ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  android
  EGL
  GLESv3
  log
)
