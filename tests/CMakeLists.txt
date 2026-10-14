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

find_package(${QT_PACKAGE} REQUIRED COMPONENTS Test)

# Each public header must compile as the first include in its own TU.
set(header_checks)
foreach(header IN LISTS bernese_public_headers)
  get_filename_component(name "${header}" NAME)
  file(RELATIVE_PATH source_header "${PROJECT_SOURCE_DIR}/src" "${header}")
  set(check "${CMAKE_CURRENT_BINARY_DIR}/header_checks/${name}.cc")
  file(GENERATE
    OUTPUT "${check}"
    CONTENT "#include \"${source_header}\"\n#include \"QWidget\"\n"
  )
  list(APPEND header_checks "${check}")
endforeach()

add_library(bernese-header-checks OBJECT ${header_checks})
target_link_libraries(bernese-header-checks PRIVATE bernese)

add_executable(bernese-tests
  bernese_test/bernese_test.cc
  tag_object/tag_object.h
)
target_include_directories(bernese-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(bernese-tests PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)
target_link_libraries(bernese-tests PRIVATE bernese ${QT_PACKAGE}::Test)

add_test(NAME bernese-helpers COMMAND bernese-tests)
set_tests_properties(bernese-helpers PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
if(BERNESE_QT_MAJOR_VERSION STREQUAL "6")
  add_test(NAME public-api-baseline
    COMMAND ${CMAKE_COMMAND}
      -DLIBRARY=$<TARGET_FILE:bernese>
      -DBASELINE=${CMAKE_CURRENT_SOURCE_DIR}/public_api_qt6.txt
      -P ${CMAKE_CURRENT_SOURCE_DIR}/check-public-api.cmake
  )
endif()
add_test(NAME no-qt-private-api
  COMMAND ${CMAKE_COMMAND}
    -DLIBRARY=$<TARGET_FILE:bernese>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check-qt-public.cmake
)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_custom_target(check-style
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/check_style.py
      --build-dir ${PROJECT_BINARY_DIR}
    DEPENDS bernese-tests
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    USES_TERMINAL
  )
endif()

add_executable(bernese-window-geometry-tests
  window_geometry_test.cc
)

set_target_properties(bernese-window-geometry-tests PROPERTIES AUTOMOC ON)

target_link_libraries(bernese-window-geometry-tests
  PRIVATE bernese ${QT_PACKAGE}::Test
)

add_test(NAME window-geometry
  COMMAND bernese-window-geometry-tests
)

set_tests_properties(window-geometry PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=2"
)

if(BERNESE_QT_MAJOR_VERSION STREQUAL "6" AND
    Qt6_VERSION VERSION_GREATER_EQUAL "6.10"
)
  add_test(NAME window-geometry-multiscreen
    COMMAND bernese-window-geometry-tests CrossScreenGeometry)
  set_tests_properties(window-geometry-multiscreen PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen:configfile=${CMAKE_CURRENT_SOURCE_DIR}/screens.json;QT_SCALE_FACTOR=1;QT_QPA_OFFSCREEN_NO_GLX=1")
endif()
