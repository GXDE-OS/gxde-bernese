# Copyright (C) 2026 CharOfString <root@charofstring.cc>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.16)
project(gxde-bernese VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BERNESE_QT_MAJOR_VERSION 6 CACHE STRING "Qt major version (5 or 6)")
if(NOT BERNESE_QT_MAJOR_VERSION MATCHES "^[56]$")
  message(FATAL_ERROR "BERNESE_QT_MAJOR_VERSION must be 5 or 6")
endif()

set(QT_PACKAGE Qt${BERNESE_QT_MAJOR_VERSION})
find_package(${QT_PACKAGE} REQUIRED COMPONENTS Core Gui Widgets)
find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)
pkg_check_modules(XCB_RANDR REQUIRED IMPORTED_TARGET xcb-randr)

include(GNUInstallDirs)
include(CTest)

add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
