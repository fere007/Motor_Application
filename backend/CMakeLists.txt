# - Baut die statische Lib "motor_core" (für GUI & Tests)
# - Baut optional zwei CLI-Testprogramme (test_dc, test_pmsm) im Unterordner tests/

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.21)
    project(MotorBackend LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

add_library(motor_core STATIC
    src/DC_Motor_Model.cpp
    src/PMSM_Motor_Model.cpp
)

target_include_directories(motor_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

option(BUILD_BACKEND_TESTS "Baut die CLI-Testprogramme für DC und PMSM" ON)

if(BUILD_BACKEND_TESTS)
    add_executable(test_dc tests/test_main_dc.cpp)
    target_link_libraries(test_dc PRIVATE motor_core)
    target_compile_definitions(test_dc PRIVATE TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests")

    add_executable(test_pmsm tests/test_main_pmsm.cpp)
    target_link_libraries(test_pmsm PRIVATE motor_core)
    target_compile_definitions(test_pmsm PRIVATE TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()