cmake_minimum_required(VERSION 3.2)

project(hl_service_mesh C CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(REDISCPP_FLAGS "-DREDISCPP_HEADER_ONLY=ON")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall ${REDISCPP_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -Wall -DNDEBUG ${REDISCPP_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AUTH_WRITER "auth_writer")
set(AUTH_SERVICE "auth_service")
set(MESSENGER_SERVICE "messenger_service")

find_package(OpenSSL)
find_package(Threads)
find_package(ZLIB)
find_package(CppKafka REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation Net XML JSON Util Crypto NetSSL)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
link_directories("/usr/local/lib")

add_executable(${AUTH_WRITER} auth_kafka_service/main.cpp
                                config/config.cpp
                                database/database.cpp
                                database/cache.cpp
                                database/user.cpp
                                database/chat.cpp
                                database/message.cpp
                                database/user_to_chat.cpp)
target_include_directories(${AUTH_WRITER} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${AUTH_WRITER} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${AUTH_WRITER} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${AUTH_WRITER} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${AUTH_WRITER} PRIVATE
                             ${CMAKE_THREAD_LIBS_INIT}
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "rdkafka++"
                             CppKafka::cppkafka
                             "mysqlclient"
                             ZLIB::ZLIB)


add_executable(${AUTH_SERVICE} auth_service/main.cpp
                                config/config.cpp
                                database/database.cpp
                                database/cache.cpp
                                database/user.cpp
                                database/chat.cpp
                                database/message.cpp
                                database/user_to_chat.cpp)
target_include_directories(${AUTH_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${AUTH_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${AUTH_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${AUTH_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${AUTH_SERVICE} PRIVATE
                             ${CMAKE_THREAD_LIBS_INIT}
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "rdkafka++"
                             CppKafka::cppkafka
                             "mysqlclient"
                             ZLIB::ZLIB)

add_executable(${MESSENGER_SERVICE} messenger_service/main.cpp
        config/config.cpp
        database/database.cpp
        database/cache.cpp
        database/user.cpp
        database/chat.cpp
        database/message.cpp
        database/user_to_chat.cpp)
target_include_directories(${MESSENGER_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${MESSENGER_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${MESSENGER_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${MESSENGER_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${MESSENGER_SERVICE} PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${Poco_LIBRARIES}
        "PocoData"
        "PocoDataMySQL"
        "rdkafka++"
        CppKafka::cppkafka
        "mysqlclient"
        ZLIB::ZLIB)
