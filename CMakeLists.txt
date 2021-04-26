cmake_minimum_required(VERSION 3.9)
project(Json2XML VERSION 0.0.1 DESCRIPTION "Json2XML library")

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/local/${CMAKE_BUILD_TYPE}/")
else()
    set(CMAKE_INSTALL_PREFIX "/usr/local/${CMAKE_BUILD_TYPE}/")
endif()

include_directories(BEFORE "${CMAKE_INSTALL_PREFIX}/include/")
link_directories(BEFORE "${CMAKE_INSTALL_PREFIX}/lib/")

set (SOURCES
    attributeimpl.cpp
    attributelistimpl.cpp
    json2xml.cpp
    ParseHandler.cpp
    SAXPrintHandlers.cpp
)

set (HEADERS
    attributeimpl.hpp
    attributelistimpl.hpp
    json2xml.hpp
    ParseHandler.hpp
    SAXPrint.hpp
    SAXPrintHandlers.hpp
)

add_library(Json2XML STATIC ${SOURCES} ${HEADERS})
target_compile_features(Json2XML PUBLIC cxx_std_20)
set_target_properties(Json2XML PROPERTIES
    PUBLIC_HEADER json2xml.hpp)

install(TARGETS Json2XML
    LIBRARY       DESTINATION "${CMAKE_BUILD_TYPE}/lib"
    ARCHIVE       DESTINATION "${CMAKE_BUILD_TYPE}/lib"
    PUBLIC_HEADER DESTINATION "${CMAKE_BUILD_TYPE}/include")

set(APP_EXE Json2XML_Test)
add_executable(${APP_EXE} test.cpp)
target_compile_features(${APP_EXE} PUBLIC cxx_std_20)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DFLAG "D")
else()
    set(DFLAG "")
endif()

if (WIN32)
    target_link_libraries(${APP_EXE} PRIVATE
        Json2XML
        JsonX
        xerces-c_3${DFLAG}
    )
    add_custom_command(TARGET ${APP_EXE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_INSTALL_PREFIX}/bin/xerces-c_3_2${DFLAG}.dll"
            $<TARGET_FILE_DIR:Json2XML>)
else()
    target_link_libraries(${APP_EXE} PRIVATE
        Json2XML
        JsonX
        xerces-c
        stdc++
        m)
endif()


add_test(Test ${APP_EXE})
