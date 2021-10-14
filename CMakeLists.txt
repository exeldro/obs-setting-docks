project(setting-docks VERSION 0.0.3)
set(PROJECT_FULL_NAME "Setting Docks")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

set(setting-docks_HEADERS
	setting-docks.hpp
	stream-dock.hpp
	video-dock.hpp
	bitrate-dock.hpp
	version.h)
set(setting-docks_SOURCES
	setting-docks.cpp
	stream-dock.cpp
	video-dock.cpp
	bitrate-dock.cpp)

if(WIN32)
	get_filename_component(ISS_FILES_DIR "${CMAKE_BINARY_DIR}\\..\\package" ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_FILES_DIR}" ISS_FILES_DIR)
	get_filename_component(ISS_PACKAGE_DIR "${CMAKE_PACKAGE_PREFIX}\\.." ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_PACKAGE_DIR}" ISS_PACKAGE_DIR)
	get_filename_component(ISS_SOURCE_DIR "${PROJECT_SOURCE_DIR}" ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_SOURCE_DIR}" ISS_SOURCE_DIR)
	configure_file("installer.iss.in"
			"${PROJECT_BINARY_DIR}/installer.iss"
		)
	configure_file(resource.rc.in setting-docks.rc)
	list(APPEND setting-docks_SOURCES
		setting-docks.rc)
endif()

add_library(setting-docks MODULE
	${setting-docks_HEADERS}
	${setting-docks_SOURCES})

target_link_libraries(setting-docks
		obs-frontend-api
		Qt5::Widgets
		libobs)

set_target_properties(setting-docks PROPERTIES FOLDER "plugins/exeldro")
install_obs_plugin_with_data(setting-docks data)
