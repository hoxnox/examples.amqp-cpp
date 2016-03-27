cmake_minimum_required(VERSION 2.8)
option(WITH_SYSTEM_LIBEVENT "Don't build libevent, use installed" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
add_definitions("-std=c++11")

########################################################################
# RabbitMQ

# too complicated
#    include(ExternalProject)
#    if (WITH_SYSTEM_RABBITMQ)
#        find_package(RABBITMQ REQUIRED)
#        add_custom_target(rabbitmq DEPENDS ${RABBITMQ_INCLUDE_DIR}/amqp.h)
#    else()
#        ExternalProject_Add(rabbitmq
#            URL "https://github.com/alanxz/rabbitmq-c/archive/v0.7.1.tar.gz"
#            URL_MD5 "c40427844e7a2b94f9687fa7606829ee"
#            CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
#            INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/rabbitmq"
#        )
#        set(RABBITMQ_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/rabbitmq/include)
#        set(RABBITMQ_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/rabbitmq/lib64/librabbitmq.a)
#        message(STATUS "Found external dependency RABBITMQ: " ${RABBOTMQ_INCLUDE_DIR})
#    endif()
#    include_directories(${RABBOTMQ_INCLUDE_DIR})
#    list(APPEND LIBRARIES ${RABBITMQ_LIBRARIES})
#    list(APPEND DEPENDENCIES rabbitmq)

include(ExternalProject)
set(AMQPCPP_ROOT "${CMAKE_CURRENT_BINARY_DIR}/amqpcpp")
ExternalProject_Add(amqpcpp
    URL "https://github.com/hoxnox/AMQP-CPP/archive/v2.5.1-nx1.tar.gz"
    URL_MD5 "0769867d464f1c30d4fed9016c8b6b34"
    CMAKE_ARGS -Wno-dev "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    INSTALL_DIR ${AMQPCPP_ROOT}
)
set(AMQPCPP_INCLUDE_DIR ${AMQPCPP_ROOT}/include)
add_library(amqpcpp_lib STATIC IMPORTED)
set_property(TARGET amqpcpp_lib PROPERTY IMPORTED_LOCATION ${AMQPCPP_ROOT}/lib/libamqp-cpp.a)
set(AMQPCPP_LIBRARIES amqpcpp_lib)
message(STATUS "Found external dependency AMQP-CPP: " ${AMQPCPP_INCLUDE_DIR})
include_directories(${AMQPCPP_INCLUDE_DIR})
list(APPEND LIBRARIES ${AMQPCPP_LIBRARIES})
list(APPEND DEPENDENCIES amqpcpp)

########################################################################
# libevent

if (WITH_SYSTEM_LIBEVENT)
    find_package(Event REQUIRED)
    add_custom_target(event DEPENDS ${Event_INCLUDE_DIR}/event2/event.h)
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
    add_library(event_core_lib STATIC IMPORTED)
    set_property(TARGET event_core_lib PROPERTY IMPORTED_LOCATION ${Event_ROOT}/lib/libevent_core.a)
    set(Event_LIBRARIES event_core_lib)
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
#executable(consumer consumer.cpp)
#et_link_libraries(producer ${LIBRARIES})
#dependencies(producer ${DEPENDENCIES})
