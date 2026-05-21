cmake_minimum_required(VERSION 3.20)

project(modular_protocols LANGUAGES CXX)

add_executable(modular_protocols
    src/main.cpp
 "include/ProtocolInterface.h" "include/ComInterface.h" "src/ComInterface.cpp" "include/UdpInterface.h" "src/UdpInterface.cpp")

target_compile_definitions(modular_protocols PRIVATE 
    ASIO_STANDALONE 
)

target_include_directories(modular_protocols
    PRIVATE
        include
        $ENV{Dependencies}/asio-1.36.0/include 
)

target_compile_features(modular_protocols PRIVATE cxx_std_20)

if(WIN32)
    target_link_libraries(modular_protocols PRIVATE 
        ws2_32 
        mswsock 
    )
endif()
