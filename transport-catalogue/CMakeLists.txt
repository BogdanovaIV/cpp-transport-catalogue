# CMakeList.txt: проект CMake для PersonTest; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required(VERSION 3.10)
project(transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto)
# Добавьте источник в исполняемый файл этого проекта.
add_executable(transport_catalogue
domain.cpp
domain.h
geo.cpp
geo.h
graph.h
json.cpp
json.h
json_builder.cpp
json_builder.h
json_reader.cpp
json_reader.h
main.cpp
map_renderer.cpp
map_renderer.h
ranges.h
request_handler.cpp
request_handler.h
router.h
serialization.cpp
serialization.h
svg.cpp
svg.h
transport_catalogue.cpp
transport_catalogue.h
transport_router.cpp
transport_router.h
${PROTO_SRCS}
${PROTO_HDRS})


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET transport_catalogue PROPERTY CXX_STANDARD 20)
endif()


target_link_libraries(transport_catalogue ${Protobuf_LIBRARIES})
