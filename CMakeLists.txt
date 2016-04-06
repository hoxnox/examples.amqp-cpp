cmake_minimum_required(VERSION 2.8)
option(WITH_SYSTEM_LIBEVENT "Don't build libevent, use installed" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
add_definitions("-std=c++11")

########################################################################
# RabbitMQ

include(ExternalProject)
set(AMQPCPP_ROOT "${CMAKE_CURRENT_BINARY_DIR}/amqpcpp")
ExternalProject_Add(libamqpcpp
    URL "https://github.com/hoxnox/AMQP-CPP/archive/v2.5.1-nx2.tar.gz"
    URL_MD5 "fcfbd25c03eecde4e4b0dfa58598a426"
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

add_executable(receive receive.cpp)
target_link_libraries(receive ${LIBRARIES})
add_dependencies(receive ${DEPENDENCIES})
add_executable(send send.cpp)
target_link_libraries(send ${LIBRARIES})
add_dependencies(send ${DEPENDENCIES})

add_executable(worker worker.cpp)
target_link_libraries(worker ${LIBRARIES})
add_dependencies(worker ${DEPENDENCIES})
add_executable(new_task new_task.cpp)
target_link_libraries(new_task ${LIBRARIES})
add_dependencies(new_task ${DEPENDENCIES})

add_executable(receive_logs receive_logs.cpp)
target_link_libraries(receive_logs ${LIBRARIES})
add_dependencies(receive_logs ${DEPENDENCIES})
add_executable(emit_log emit_log.cpp)
target_link_libraries(emit_log ${LIBRARIES})
add_dependencies(emit_log ${DEPENDENCIES})

add_executable(receive_logs_direct receive_logs_direct.cpp)
target_link_libraries(receive_logs_direct ${LIBRARIES})
add_dependencies(receive_logs_direct ${DEPENDENCIES})
add_executable(emit_log_direct emit_log_direct.cpp)
target_link_libraries(emit_log_direct ${LIBRARIES})
add_dependencies(emit_log_direct ${DEPENDENCIES})

add_executable(receive_logs_topic receive_logs_topic.cpp)
target_link_libraries(receive_logs_topic ${LIBRARIES})
add_dependencies(receive_logs_topic ${DEPENDENCIES})
add_executable(emit_log_topic emit_log_topic.cpp)
target_link_libraries(emit_log_topic ${LIBRARIES})
add_dependencies(emit_log_topic ${DEPENDENCIES})

add_executable(rpc_server rpc_server.cpp)
target_link_libraries(rpc_server ${LIBRARIES})
add_dependencies(rpc_server ${DEPENDENCIES})
add_executable(rpc_client rpc_client.cpp)
target_link_libraries(rpc_client ${LIBRARIES})
add_dependencies(rpc_client ${DEPENDENCIES})
