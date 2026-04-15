cmake_minimum_required(VERSION 3.16)

project(SimsModFixer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Let callers provide Qt with -DCMAKE_PREFIX_PATH, -DQt6_DIR, or QTDIR.
if(NOT CMAKE_PREFIX_PATH AND DEFINED ENV{QTDIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

if(WIN32)
    add_executable(SimsModFixer WIN32 ${PROJECT_SOURCES})
elseif(APPLE)
    add_executable(SimsModFixer MACOSX_BUNDLE ${PROJECT_SOURCES})
else()
    add_executable(SimsModFixer ${PROJECT_SOURCES})
endif()

set_target_properties(SimsModFixer PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "SimsModFixer"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.pisu.simsmodfixer"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

target_link_libraries(SimsModFixer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(WIN32)
    get_target_property(_qt_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET SimsModFixer POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    --no-opengl-sw
                    --verbose 0
                    "$<TARGET_FILE:SimsModFixer>"
            COMMENT "Deploying Qt runtime with windeployqt"
        )
    else()
        message(WARNING "windeployqt not found. App may fail to start outside Qt-configured environments.")
    endif()
endif()

if(APPLE)
    get_target_property(_qt_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake_executable}" DIRECTORY)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET SimsModFixer POST_BUILD
            COMMAND "${MACDEPLOYQT_EXECUTABLE}"
                    "$<TARGET_BUNDLE_DIR:SimsModFixer>"
                    -verbose=0
            COMMENT "Deploying Qt runtime with macdeployqt"
        )
    else()
        message(WARNING "macdeployqt not found. App bundle may fail to start outside Qt-configured environments.")
    endif()
endif()
