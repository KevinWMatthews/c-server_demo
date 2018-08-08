add_executable(server_unix
    server.c
    socket_unix.h
    socket_unix.c
)
add_executable(client_unix
    client.c
    socket_unix.h
    socket_unix.c
)

add_executable(server_client_unix
    server_and_client.c
)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
target_link_libraries(server_client_unix Threads::Threads)
