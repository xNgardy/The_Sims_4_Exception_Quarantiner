cmake_minimum_required(VERSION 3.16)

project(SimsModFixer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

# DİKKAT: Eğik çizgiler düzeltildi (/). 
# Eğer VS Code'da MSVC seçtiysen, buradaki mingw_64'ü msvc..._64 olarak değiştirmelisin!
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/msvc2022_64" CACHE PATH "Qt Install Path")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

add_executable(SimsModFixer WIN32 ${PROJECT_SOURCES})

# Show console in Debug to surface startup errors; hide it in non-Debug configs.
set_target_properties(SimsModFixer PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

target_link_libraries(SimsModFixer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Deploy required Qt runtime DLLs next to the executable on Windows.
if (WIN32)
    get_target_property(_qt_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    if (WINDEPLOYQT_EXECUTABLE)
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