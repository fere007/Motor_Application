cmake_minimum_required(VERSION 3.21)
project(Motor_Application VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup()

add_subdirectory(backend)

qt_add_executable(Motor_Application
    main.cpp
)

qt_add_qml_module(Motor_Application
    URI MotorApp
    VERSION 1.0
    SOURCES
        bridge/MotorBackend.h
        bridge/MotorBackend.cpp
    QML_FILES
        frontend/Main.qml
)

target_link_libraries(Motor_Application PRIVATE
    motor_core
    Qt6::Quick
)