#https://www.kdab.com/using-cmake-with-qt-5/

cmake_minimum_required(VERSION 3.8)

# Project Information
project (grimgal VERSION 0.1.0)

# Set compiler options
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_COMPILER             "/usr/bin/clang")
set (CMAKE_C_FLAGS                "-fPIC -Weverything -pthread")
set (CMAKE_C_FLAGS_DEBUG          "-g")
set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
set (CMAKE_CXX_FLAGS                "-fPIC -Weverything -Wno-c++98-compat -Wno-padded -Wno-covered-switch-default -pthread")
set (CMAKE_CXX_FLAGS_DEBUG          "-g")
set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

set (CMAKE_SHARED_LINKER_FLAGS "-v -lboost_system -lboost_regex")

find_package(Boost 1.65.0 REQUIRED COMPONENTS system filesystem regex)

find_package(Qt5 REQUIRED Widgets Xml)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

include_directories(
	"src/"
)

# Source
set (SOURCES

	src/main.cpp

	# Data
	src/data/probe.cpp
	src/data/project.cpp
	src/data/measurement.cpp

	# Form
	src/form/mainwindow.cpp
	src/form/settings_dialog.cpp
	#form/addmeasurmentwizard.cpp

	# Model
	src/model/eventtablemodel.cpp
	src/model/measurementtreemodel.cpp
	src/model/probetablemodel.cpp
	src/model/propertytablemodel.cpp
	src/model/statistictablemodel.cpp
	src/model/settings_dialog_color_model.cpp

	# Widget
	src/widget/customqglwidget.cpp

	# Util
	src/util/number_format.cpp

	# EventFilter
	src/eventfilter/probeview/removeprobe.cpp
)

set (UIS
	form/mainwindow.ui
	form/settings_dialog.ui
	#form/addmeasurmentwizard.ui
)

set (RESOURCES
	grimgal.qrc
)

qt5_wrap_ui(UI_HEADERS ${UIS})
add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCES} ${UI_HEADERS})

qt5_use_modules(${PROJECT_NAME} Core Widgets Xml)
target_link_libraries(${PROJECT_NAME} readerlib_s)
