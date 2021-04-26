cmake_minimum_required(VERSION 3.9)
project(Json2XML VERSION 0.0.1 DESCRIPTION "Json2XML library")

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(BEFORE "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/")
link_directories(BEFORE "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib/")

set (Json2XML_SOURCES
    attributeimpl.cpp
    attributelistimpl.cpp
    json2xml.cpp
    ParseHandler.cpp
    SAXPrintHandlers.cpp
)

set (Json2XML_HEADERS
    attributeimpl.hpp
    attributelistimpl.hpp
    json2xml.hpp
    ParseHandler.hpp
    SAXPrint.hpp
    SAXPrintHandlers.hpp
)

add_library(Json2XML STATIC ${Json2XML_SOURCES} ${Json2XML_HEADERS})
target_compile_features(Json2XML PUBLIC cxx_std_20)
set_target_properties(Json2XML PROPERTIES
    PUBLIC_HEADER json2xml.hpp)

add_executable(Json2XML_Test test.cpp)
target_compile_features(Json2XML_Test PUBLIC cxx_std_20)

add_executable(j2x j2x.cpp)
target_compile_features(j2x PUBLIC cxx_std_20)

install(TARGETS Json2XML j2x
    LIBRARY       DESTINATION "${CMAKE_BUILD_TYPE}/lib"
    ARCHIVE       DESTINATION "${CMAKE_BUILD_TYPE}/lib"
    RUNTIME       DESTINATION "${CMAKE_BUILD_TYPE}/bin"
    PUBLIC_HEADER DESTINATION "${CMAKE_BUILD_TYPE}/include")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DFLAG "D")
else()
    set(DFLAG "")
endif()

if (WIN32)
    target_link_libraries(Json2XML_Test PRIVATE
        Json2XML
        JsonX
        xerces-c_3${DFLAG}
    )
    add_custom_command(TARGET Json2XML_Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/bin/xerces-c_3_2${DFLAG}.dll"
            $<TARGET_FILE_DIR:Json2XML>)

    target_link_libraries(j2x PRIVATE
        Json2XML
        JsonX
        xerces-c_3${DFLAG}
    )
    add_custom_command(TARGET j2x POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/bin/xerces-c_3_2${DFLAG}.dll"
            $<TARGET_FILE_DIR:j2x>)

else()
    target_link_libraries(Json2XML_Test PRIVATE
        Json2XML
        JsonX
        xerces-c
        stdc++
        m
    )
endif()


add_test(Test Json2XML_Test)
