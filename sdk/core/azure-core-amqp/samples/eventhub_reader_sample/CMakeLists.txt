cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(eventhub_reader_sample)

# Unit Tests
add_executable(eventhub_reader_sample
    eventhub_reader_sample.cpp
)
add_dependencies(eventhub_reader_sample azure-core-amqp)

target_include_directories(eventhub_reader_sample PRIVATE ${azure-core-amqp_SOURCE_DIR})

# Link test executable against the prototype.
target_link_libraries(eventhub_reader_sample PRIVATE azure-core-amqp)
