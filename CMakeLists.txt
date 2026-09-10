cmake_minimum_required(VERSION 3.21)
project(Spaste VERSION 1.0.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Svg
    SvgWidgets
    Network
)

find_package(X11 REQUIRED)
find_library(X11_Xtst_LIB Xtst REQUIRED)
find_path(X11_XTest_INCLUDE_PATH X11/extensions/XTest.h REQUIRED)

qt_add_executable(spaste
    src/main.cpp
    src/app/Application.cpp
    src/app/Application.h
    src/app/SingleInstance.cpp
    src/app/SingleInstance.h
    src/clipboard/ClipboardHistory.cpp
    src/clipboard/ClipboardHistory.h
    src/hotkeys/DesktopShortcut.cpp
    src/hotkeys/DesktopShortcut.h
    src/hotkeys/GlobalHotkey.cpp
    src/hotkeys/GlobalHotkey.h
    src/paste/AutoPaster.cpp
    src/paste/AutoPaster.h
    src/theme/ThemeManager.cpp
    src/theme/ThemeManager.h
    src/ui/SettingsWindow.cpp
    src/ui/SettingsWindow.h
    src/ui/OverlayWindow.cpp
    src/ui/OverlayWindow.h
    src/ui/ClipboardItemWidget.cpp
    src/ui/ClipboardItemWidget.h
    resources/resources.qrc
)

target_include_directories(spaste PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${X11_INCLUDE_DIR}
    ${X11_XTest_INCLUDE_PATH}
)

target_link_libraries(spaste PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Network
    ${X11_LIBRARIES}
    ${X11_Xtst_LIB}
)

target_compile_definitions(spaste PRIVATE
    SPASTE_VERSION="${PROJECT_VERSION}"
)

include(GNUInstallDirs)

install(TARGETS spaste RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/spaste.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES resources/icons/clipboard.svg
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
        RENAME spaste.svg)

# Debian package via CPack
# SHLIBDEPS is off on purpose: Ubuntu 24.04 emits *t64 package names that are
# not installable on Debian / Ubuntu 22.04 / many derivatives (e.g. Alfa OS).
set(CPACK_PACKAGE_NAME "spaste")
set(CPACK_PACKAGE_VENDOR "Spaste")
set(CPACK_PACKAGE_CONTACT "Spaste maintainers <spaste@users.noreply.github.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lightweight RAM-only clipboard history for Linux")
set(CPACK_PACKAGE_DESCRIPTION "Spaste is a Qt 6 clipboard history manager. Launching the app opens settings; Super+V shows a frameless overlay with the 10 most recent clipboard items. History is kept in memory only and is never written to disk.")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/hdmain/superpaster")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6core6 | libqt6core6t64, libqt6gui6 | libqt6gui6t64, libqt6widgets6 | libqt6widgets6t64, libqt6svg6 | libqt6svg6t64, libqt6svgwidgets6 | libqt6svgwidgets6t64, libqt6network6 | libqt6network6t64, qt6-qpa-plugins, libx11-6, libxtst6, libc6, libstdc++6, libgcc-s1")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "xdotool | wtype")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Spaste maintainers <spaste@users.noreply.github.com>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)
