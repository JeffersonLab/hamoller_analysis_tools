cmake_minimum_required(VERSION 3.10)
project(Pedestal)

find_package(ROOT REQUIRED COMPONENTS RIO Tree Hist)

include(${ROOT_USE_FILE})

include_directories(include)

set(SOURCES
    Pedestal.cxx
)

set(HEADERS
    Pedestal.h
)

ROOT_GENERATE_DICTIONARY(G__Pedestal
    ${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS}
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h
)

add_library(Pedestal SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCES}
    G__Pedestal.cxx
)

target_link_libraries(Pedestal ${ROOT_LIBRARIES})
