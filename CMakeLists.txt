#    This file is part of show_stixels. (https://github.com/dhernandez0/show_stixels).
#
#    Copyright (c) 2016 Daniel Hernandez Juarez.

#    show_stixels is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    show_stixels is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with show_stixels.  If not, see <http://www.gnu.org/licenses/>.

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
