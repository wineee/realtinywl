find_package(Qt6 COMPONENTS Core Quick QuickControls2 REQUIRED)
qt_standard_project_setup(REQUIRES 6.6)

if(QT_KNOWN_POLICY_QTP0001) # this policy was introduced in Qt 6.5
    qt_policy(SET QTP0001 NEW)
    # the RESOURCE_PREFIX argument for qt_add_qml_module() defaults to ":/qt/qml/"
endif()
if(POLICY CMP0071)
    # https://cmake.org/cmake/help/latest/policy/CMP0071.html
    cmake_policy(SET CMP0071 NEW)
endif()

set(QML_IMPORT_PATH ${CMAKE_BINARY_DIR}/src/server/ CACHE STRING "" FORCE)

find_package(Waylib REQUIRED Server)
find_package(PkgConfig REQUIRED)
pkg_search_module(PIXMAN REQUIRED IMPORTED_TARGET pixman-1)
pkg_search_module(WAYLAND REQUIRED IMPORTED_TARGET wayland-server)

add_executable(tinywl
    main.cpp
)

set_source_files_properties(QmlHelper.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(tinywl
    URI Tinywl
    VERSION "1.0"
    SOURCES
        helper.h
    QML_FILES
        Main.qml
        StackWorkspace.qml
        XdgSurface.qml
        TiledWorkspace.qml
        QmlHelper.qml
        OutputDelegate.qml
        StackToplevelHelper.qml
        TiledToplevelHelper.qml


)

target_compile_definitions(tinywl
    PRIVATE
    SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    WLR_USE_UNSTABLE
)

target_link_libraries(tinywl
    PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Waylib::WaylibServer
    PkgConfig::PIXMAN
    PkgConfig::WAYLAND
)

install(TARGETS tinywl DESTINATION ${CMAKE_INSTALL_BINDIR})
