cmake_minimum_required(VERSION 3.10)
project(FileSharing)

# Set C standard to C11
set(CMAKE_C_STANDARD 11)

# Define server and client executables
add_executable(server server/server.c)
add_executable(client client/client.c)

# Specify include directories
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/server)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/client)

# Link pthreads library for multi-threading (for C, use -lpthread)
target_link_libraries(server PRIVATE pthread)

# Specify installation directory (optional)
install(TARGETS server client DESTINATION bin)

# Print build information (optional)
message("CMake build configuration:")
message(" - CMake version: ${CMAKE_VERSION}")
message(" - Project name: ${PROJECT_NAME}")
message(" - C compiler: ${CMAKE_C_COMPILER}")
message(" - Source directory: ${CMAKE_SOURCE_DIR}")
message(" - Build directory: ${CMAKE_BINARY_DIR}")


