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

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

set(bernese_public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/image_utils/image_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/text_layout_utils/text_layout_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/file_dialog_utils/file_dialog_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/x11_info/x11_info.h
  ${CMAKE_CURRENT_SOURCE_DIR}/window_utils/window_geometry.h
  ${CMAKE_CURRENT_SOURCE_DIR}/window_utils/window_stack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/window_utils/window_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/window_effects/window_effects.h
  ${CMAKE_CURRENT_SOURCE_DIR}/backdrop_origin/backdrop_origin.h
)

set(bernese_public_headers ${bernese_public_headers} PARENT_SCOPE)

set(protocol_dir ${CMAKE_CURRENT_BINARY_DIR}/wayland/protocols)
set(protocol_sources)

foreach(protocol IN ITEMS blur dde-shell)
  set(xml ${CMAKE_CURRENT_SOURCE_DIR}/window_effects/protocols/${protocol}.xml)
  set(header ${protocol_dir}/${protocol}-client-protocol.h)
  set(code ${protocol_dir}/${protocol}-protocol.c)
  add_custom_command(
    OUTPUT ${header} ${code}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${protocol_dir}
    COMMAND ${WAYLAND_SCANNER} client-header ${xml} ${header}
    COMMAND ${WAYLAND_SCANNER} private-code ${xml} ${code}
    DEPENDS ${xml}
    VERBATIM
  )
  list(APPEND protocol_sources ${header} ${code})
endforeach()

add_library(bernese SHARED
  image_utils/image_utils.cc
  text_layout_utils/text_layout_utils.cc
  file_dialog_utils/file_dialog_utils.cc
  x11_info/x11_info.cc
  window_utils/window_geometry.cc
  window_utils/window_stack.cc
  window_utils/window_utils.cc
  window_effects/window_effects.cc
  window_effects/wayland_effects.cc
  backdrop_origin/backdrop_origin.cc
  ${protocol_sources}
)
add_library(GxdeBernese::Qt${BERNESE_QT_MAJOR_VERSION} ALIAS bernese)

set_target_properties(bernese PROPERTIES
  OUTPUT_NAME gxde-bernese-qt${BERNESE_QT_MAJOR_VERSION}
  EXPORT_NAME Qt${BERNESE_QT_MAJOR_VERSION}
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN YES
)

generate_export_header(bernese
  EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/generated/bernese/export.h
)

set_target_properties(bernese PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)

target_include_directories(bernese PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/gxde-bernese-qt${BERNESE_QT_MAJOR_VERSION}>
)

target_include_directories(bernese PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/wayland)

# QGuiApplication::platformNativeInterface() and the wl_surface helper are
# declared in QtGui's private headers.
target_include_directories(bernese PRIVATE
  ${${QT_PACKAGE}Gui_PRIVATE_INCLUDE_DIRS}
)

target_compile_features(bernese PUBLIC cxx_std_20)
target_link_libraries(bernese
  PUBLIC
    ${QT_PACKAGE}::Core
    ${QT_PACKAGE}::Gui
    ${QT_PACKAGE}::Widgets
    X11::X11
    PkgConfig::XCB
  PRIVATE
    PkgConfig::XCB_RANDR
    PkgConfig::WAYLAND_CLIENT
)

set(package GxdeBerneseQt${BERNESE_QT_MAJOR_VERSION})
set(config_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${package})
set(header_dir ${CMAKE_INSTALL_INCLUDEDIR}/gxde-bernese-qt${BERNESE_QT_MAJOR_VERSION})
set(pkg_config_file gxde-bernese-qt${BERNESE_QT_MAJOR_VERSION}.pc)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/GxdeBerneseConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake
  INSTALL_DESTINATION ${config_dir}
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion
)

configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/gxde-bernese.pc.in
  ${pkg_config_file}
  @ONLY
)

install(TARGETS bernese EXPORT GxdeBerneseTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${bernese_public_headers}
  DESTINATION ${header_dir}/bernese
)

install(FILES ${PROJECT_BINARY_DIR}/generated/bernese/export.h
  DESTINATION ${header_dir}/bernese
)

install(EXPORT GxdeBerneseTargets
  NAMESPACE GxdeBernese::
  DESTINATION ${config_dir}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake
  DESTINATION ${config_dir}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${pkg_config_file}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
  DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/gxde-bernese
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/docs/man/
  DESTINATION ${CMAKE_INSTALL_MANDIR}
  COMPONENT Documentation
  FILES_MATCHING PATTERN "*.3"
)

# Distribution copies are not used to compile the library or tests.
# This target runs with the default build, but bernese does not depend on it.
add_custom_target(dist-headers ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
    ${PROJECT_BINARY_DIR}/include/bernese
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${bernese_public_headers}
    ${PROJECT_BINARY_DIR}/generated/bernese/export.h
    ${PROJECT_BINARY_DIR}/include/bernese
  VERBATIM
)
