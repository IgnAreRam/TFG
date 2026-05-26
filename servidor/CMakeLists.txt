cmake_minimum_required(VERSION 3.10)
project(elcentinela_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# PostgreSQL (libpq) y pthreads
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

# Todas las clases viven en modelo/, controladora/ y vista/.
file(GLOB FUENTES
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/modelo/*.cpp"
    "${CMAKE_SOURCE_DIR}/controladora/*.cpp"
    "${CMAKE_SOURCE_DIR}/vista/*.cpp"
)

add_executable(elcentinela_server ${FUENTES})

# Así cada clase puede incluir las demás con "Clase.h" sin rutas relativas feas.
target_include_directories(elcentinela_server PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/modelo
    ${CMAKE_SOURCE_DIR}/controladora
    ${CMAKE_SOURCE_DIR}/vista
    ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(elcentinela_server
    PostgreSQL::PostgreSQL
    Threads::Threads
)
