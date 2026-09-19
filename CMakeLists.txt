cmake_minimum_required(VERSION 3.20)

project(SudokuImageSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

add_executable(SudokuImageSolver
    MainTest.cpp
)

target_include_directories(SudokuImageSolver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
)

target_link_libraries(SudokuImageSolver PRIVATE
    ${OpenCV_LIBS}
    Tesseract::libtesseract
)