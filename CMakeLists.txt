cmake_minimum_required(VERSION 2.4)
project(show_stixels)
find_package( OpenCV REQUIRED )

add_executable(show_stixels vDisparity.cpp show_stixels.cpp)

install(
  TARGETS show_stixels
  RUNTIME DESTINATION bin
)

target_link_libraries( show_stixels ${OpenCV_LIBS} )

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wextra -Winline -Werror -pedantic -pedantic-errors -march=native -O3 -ffast-math")
