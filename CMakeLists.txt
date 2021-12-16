project(setting-docks)

add_library(setting-docks MODULE)
add_library(OBS::setting-docks ALIAS setting-docks)

if(BUILD_QT EQUAL 5)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
  
  target_link_libraries(setting-docks PRIVATE Qt5::Widgets Qt5::GuiPrivate)

  set_target_properties(
    setting-docks
    PROPERTIES AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON
               AUTOUIC_SEARCH_PATHS "media")
			   
  target_include_directories(
  setting-docks PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
							   
elseif(BUILD_QT EQUAL 6)
  message(FATAL_ERROR "OBS: Building with Qt6 not supported yet.")
endif()

target_sources(setting-docks PRIVATE setting-docks.hpp
	stream-dock.hpp
	video-dock.hpp
	bitrate-dock.hpp
	version.h
	setting-docks.cpp
	stream-dock.cpp
	video-dock.cpp
	bitrate-dock.cpp)

target_link_libraries(setting-docks PRIVATE OBS::libobs OBS::frontend-api)

target_compile_features(setting-docks PRIVATE cxx_std_17)

set_target_properties(setting-docks PROPERTIES FOLDER "plugins/exeldro" PREFIX "")

setup_plugin_target(setting-docks)
