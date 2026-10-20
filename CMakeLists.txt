cmake_minimum_required(VERSION 3.16)
project(SDR C CXX)

find_package(SoapySDR REQUIRED)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE ${SoapySDR_LIBRARIES})