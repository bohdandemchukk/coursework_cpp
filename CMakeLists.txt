cmake_minimum_required(VERSION 3.16)

project(ImageEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

set(PROJECT_SOURCES
        main.cpp
        


)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ImageEditor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ui/MainWindow.h
	ui/MainWindow.cpp
        graphics/MyGraphicsView.h
        graphics/MyGraphicsView.cpp
    core/filters/RotateFilter.h core/filters/RotateFilter.cpp
    core/filters/FlipFilter.h core/filters/FlipFilter.cpp
    core/filters/BWFilter.h core/filters/BWFilter.cpp
    core/filters/BrightnessFilter.h core/filters/BrightnessFilter.cpp
    core/filters/saturationfilter.h core/filters/saturationfilter.cpp
    core/filters/contrastfilter.h core/filters/contrastfilter.cpp
    core/filters/gaussianblurutil.h core/filters/gaussianblurutil.cpp
    core/filters/blurfilter.h core/filters/blurfilter.cpp
    core/filters/sharpenfilter.h core/filters/sharpenfilter.cpp
    core/pipeline/filterpipeline.h core/pipeline/filterpipeline.cpp

    core/filters/imagefilter.h
    core/filters/temperaturefilter.h core/filters/temperaturefilter.cpp
    core/filters/exposurefilter.h core/filters/exposurefilter.cpp
    core/filters/gammafilter.h core/filters/gammafilter.cpp
    core/filters/tintfilter.h core/filters/tintfilter.cpp
    core/filters/vibrancefilter.h core/filters/vibrancefilter.cpp
    core/filters/rgbhsvutil.h core/filters/rgbhsvutil.cpp
    core/filters/shadowfilter.h core/filters/shadowfilter.cpp
    core/filters/highlightfilter.h core/filters/highlightfilter.cpp
    core/filters/clarityFilter.h core/filters/clarityFilter.cpp
    core/filters/vignettefilter.h core/filters/vignettefilter.cpp
    core/filters/grainfilter.h core/filters/grainfilter.cpp
    core/filters/splittoningfilter.h core/filters/splittoningfilter.cpp
    core/filters/fadefilter.h core/filters/fadefilter.cpp

    commands/undoredostack.h commands/undoredostack.cpp
    commands/command.h
    commands/legacy/changefilterintcommand.h commands/legacy/changefilterintcommand.cpp
    commands/legacy/changefilterboolcommand.h commands/legacy/changefilterboolcommand.cpp
    commands/cropcommand.h commands/cropcommand.cpp
    widgets/collapsiblesection.h widgets/collapsiblesection.cpp
    widgets/filterslider.h widgets/filterslider.cpp
    tools/brushtool.h tools/brushtool.cpp
    tools/erasertool.h tools/erasertool.cpp
    tools/tool.h tools/tool.cpp
    commands/strokecommand.h commands/strokecommand.cpp
    core/layers/layer.h core/layers/layer.cpp
    commands/layercommands.h commands/layercommands.cpp
    core/layers/layermanager.h core/layers/layermanager.cpp
    ui/layerspanel.h ui/layerspanel.cpp

    core/image/imageio.h core/image/imageio.cpp
    ui/filterspanel.h ui/filterspanel.cpp
    commands/rotatelayercommand.h commands/rotatelayercommand.cpp
    commands/fliplayercommand.h commands/fliplayercommand.cpp
    commands/changelayerpipelinecommand.h commands/changelayerpipelinecommand.cpp
    core/filters/fastblur.h core/filters/fastblur.cpp
    core/filters/fastblurfilter.h core/filters/fastblurfilter.cpp
    resources/icons.qrc
    resources/styles.qrc






    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ImageEditor APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ImageEditor SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ImageEditor
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(ImageEditor PRIVATE
    core
    ui
    widgets
    graphics
    commands
    tools
)

target_link_libraries(ImageEditor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ImageEditor)
endif()
set_target_properties(ImageEditor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ImageEditor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ImageEditor)
endif()
