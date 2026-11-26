cmake_minimum_required(VERSION 3.16)
project(SDR C CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

find_package(SoapySDR REQUIRED)

set(MAIN_SOURCE_FILES src/main.cpp src/tx.cpp src/sdr.cpp)
add_executable(main ${MAIN_SOURCE_FILES})
target_include_directories(main PRIVATE include)
target_link_libraries(main ${SoapySDR_LIBRARIES})

include(FetchContent)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.4)
FetchContent_Declare(implot GIT_REPOSITORY https://github.com/epezent/implot.git GIT_TAG v0.16)
FetchContent_MakeAvailable(imgui implot)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

set(LIB_SOURCE_FILES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp)
set_source_files_properties(${LIB_SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "-w")
add_executable(ui src/ui.cpp src/tx.cpp src/sdr.cpp src/dsp.cpp ${LIB_SOURCE_FILES})
target_include_directories(ui PRIVATE include)
target_include_directories(ui SYSTEM PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${implot_SOURCE_DIR})
target_compile_definitions(ui PRIVATE GL_SILENCE_DEPRECATION)
target_link_libraries(ui ${SoapySDR_LIBRARIES} SDL2::SDL2 OpenGL::GL)
