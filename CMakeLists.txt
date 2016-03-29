cmake_minimum_required(VERSION 2.8)
option(WITH_SYSTEM_LIBEVENT "Don't build libevent, use installed" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
add_definitions("-std=c++11")

########################################################################
# RabbitMQ

include(ExternalProject)
set(AMQPCPP_ROOT "${CMAKE_CURRENT_BINARY_DIR}/amqpcpp")
ExternalProject_Add(libamqpcpp
    URL "https://github.com/hoxnox/AMQP-CPP/archive/v2.5.1-nx1.tar.gz"
    URL_MD5 "0769867d464f1c30d4fed9016c8b6b34"
    CMAKE_ARGS -Wno-dev "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    INSTALL_DIR ${AMQPCPP_ROOT}
)
set(AMQPCPP_INCLUDE_DIR ${AMQPCPP_ROOT}/include)
set(AMQPCPP_LIBRARIES ${AMQPCPP_ROOT}/lib/libamqp-cpp.a)
message(STATUS "Found external dependency AMQP-CPP: " ${AMQPCPP_INCLUDE_DIR})
include_directories(${AMQPCPP_INCLUDE_DIR})
list(APPEND LIBRARIES ${AMQPCPP_LIBRARIES})
list(APPEND DEPENDENCIES libamqpcpp)

########################################################################
# libevent

if (WITH_SYSTEM_LIBEVENT)
    find_package(Event REQUIRED)
    add_custom_target(libevent DEPENDS ${Event_INCLUDE_DIR}/event2/event.h)
else()
    set(Event_ROOT "${CMAKE_CURRENT_BINARY_DIR}/libevent")
    ExternalProject_Add(libevent
        URL "https://github.com/libevent/libevent/archive/release-2.0.22-stable.tar.gz"
        URL_MD5 "8913ef56ec329f2c046007bd634c7201"
        PATCH_COMMAND <SOURCE_DIR>/autogen.sh
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --disable-shared --enable-static
            --disable-openssl --disable-libevent-regress --prefix <INSTALL_DIR>
        INSTALL_DIR ${Event_ROOT}
    )
    set(Event_INCLUDE_DIR ${Event_ROOT}/include)
    set(Event_LIBRARIES ${Event_ROOT}/lib/libevent_core.a)
    message(STATUS "Found external dependency Event: " ${Event_INCLUDE_DIR})
endif()
include_directories(${Event_INCLUDE_DIR})
list(APPEND LIBRARIES ${Event_LIBRARIES})
list(APPEND DEPENDENCIES libevent)

########################################################################
# threads

find_package(Threads)
list(APPEND LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

########################################################################

add_executable(producer producer.cpp)
target_link_libraries(producer ${LIBRARIES})
add_dependencies(producer ${DEPENDENCIES})
add_executable(consumer consumer.cpp)
target_link_libraries(consumer ${LIBRARIES})
add_dependencies(consumer ${DEPENDENCIES})

add_executable(worker worker.cpp)
target_link_libraries(worker ${LIBRARIES})
add_dependencies(worker ${DEPENDENCIES})
add_executable(tyrant tyrant.cpp)
target_link_libraries(tyrant ${LIBRARIES})
add_dependencies(tyrant ${DEPENDENCIES})
