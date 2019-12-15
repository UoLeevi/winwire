cmake_minimum_required(VERSION 3.12)
project(winwire VERSION 0.1.0)

find_package(Python REQUIRED COMPONENTS Development)

if(NOT TARGET uo::uo_linkpool)
    find_package(uo_linkpool CONFIG REQUIRED)
endif()

add_library(winwire SHARED
    winwire.c)

target_link_libraries(winwire
    PRIVATE
        uo::uo_linkpool
        Python::Python)

set_target_properties(winwire PROPERTIES PREFIX "")
set_target_properties(winwire PROPERTIES SUFFIX ".pyd")

target_compile_features(winwire
    PUBLIC
        c_std_11)

target_include_directories(winwire
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/winwire>
        $<INSTALL_INTERFACE:include/winwire>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_library(winwire::winwire ALIAS winwire)

install(TARGETS winwire EXPORT winwireTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT winwireTargets
    FILE winwireTargets.cmake
    NAMESPACE winwire::
    DESTINATION lib/cmake/winwire)

install(DIRECTORY include/winwire
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(winwireConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES winwireConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/winwireConfigVersion.cmake
    DESTINATION lib/cmake/winwire)
