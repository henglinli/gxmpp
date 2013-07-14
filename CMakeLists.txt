cmake_minimum_required (VERSION 2.8)
project(gxmpp)
include_directories(${gxmpp_SOURCE_DIR} ${gxmpp_SOURCE_DIR}/third_party/expat/lib)
if(UNIX)
  add_definitions(-DPOSIX -DLOGGING)
  if(APPLE)
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
    add_definitions(-DOSX -DLOGGING)
  else()
  endif()
endif()
add_subdirectory(third_party)
add_subdirectory(talk)
add_subdirectory(gxmpp)
add_subdirectory(examples)
add_library(gxmpp STATIC $<TARGET_OBJECTS:expat>
  $<TARGET_OBJECTS:talk_base> 
  $<TARGET_OBJECTS:talk_xmllite>
  $<TARGET_OBJECTS:talk_xmpp>
  $<TARGET_OBJECTS:extra>)