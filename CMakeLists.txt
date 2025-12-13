cmake_minimum_required(VERSION 3.16)

project(ImageEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp


)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ImageEditor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        MainWindow.h
        MyGraphicsView.h
        MyGraphicsView.cpp
        CollapsibleSection.cpp
            CollapsibleSection.h
            FilterSlider.cpp
            FilterSlider.h
    filters/RotateFilter.h filters/RotateFilter.cpp
    filters/FlipFilter.h filters/FlipFilter.cpp
    filters/BWFilter.h filters/BWFilter.cpp
    filters/BrightnessFilter.h filters/BrightnessFilter.cpp
    filters/saturationfilter.h filters/saturationfilter.cpp
    filters/contrastfilter.h filters/contrastfilter.cpp
    filters/gaussianblurutil.h filters/gaussianblurutil.cpp
    filters/blurfilter.h filters/blurfilter.cpp
    filters/sharpenfilter.h filters/sharpenfilter.cpp
    filterpipeline.h filterpipeline.cpp

    filters/imagefilter.h
    filters/temperaturefilter.h filters/temperaturefilter.cpp
    filters/exposurefilter.h filters/exposurefilter.cpp
    filters/gammafilter.h filters/gammafilter.cpp
    filters/tintfilter.h filters/tintfilter.cpp
    filters/vibrancefilter.h filters/vibrancefilter.cpp
    filters/rgbhsvutil.h filters/rgbhsvutil.cpp
    filters/shadowfilter.h filters/shadowfilter.cpp
    filters/highlightfilter.h filters/highlightfilter.cpp
    filters/clarityFilter.h filters/clarityFilter.cpp
    filters/vignettefilter.h filters/vignettefilter.cpp
    filters/grainfilter.h filters/grainfilter.cpp
    filters/splittoningfilter.h filters/splittoningfilter.cpp
    filters/fadefilter.h filters/fadefilter.cpp

    undoredostack.h undoredostack.cpp
    command.h
    changefilterintcommand.h changefilterintcommand.cpp
    changefilterboolcommand.h changefilterboolcommand.cpp
    cropcommand.h cropcommand.cpp
    collapsiblesection.h collapsiblesection.cpp
    filterslider.h filterslider.cpp
    brushtool.h brushtool.cpp
    erasertool.h erasertool.cpp
    tool.h tool.cpp
    strokecommand.h strokecommand.cpp
    layer.h layer.cpp
    layercommands.h layercommands.cpp
    layermanager.h layermanager.cpp
    layerspanel.h layerspanel.cpp
    movetool.h movetool.cpp






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

target_include_directories(ImageEditor PRIVATE filters)
target_link_libraries(ImageEditor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

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
