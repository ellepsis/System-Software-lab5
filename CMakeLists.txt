cmake_minimum_required(VERSION 3.1)
project(lab5)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -pthread -std=c11")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -pthread -std=c11")

set(SOURCE_FILES part1serv.c)
add_executable(part1serv ${SOURCE_FILES} part1serv.h part1serv.c overal_functions.h overal_functions.c)

add_executable(p1shm_client p1shm_client.h p1shm_client.c overal_functions.h
overal_functions.c part1serv.h)

add_executable(p1msg_client p1msgq_client.h p1msgq_client.c overal_functions.h
overal_functions.c part1serv.h )

add_executable(p1mmap_client p1mmap_client.h p1mmap_client.c
overal_functions.c overal_functions.h part1serv.h part7.h part7.c
part8_serv_uds.h part8_serv_uds.c part9_sig_serv.h part9_sig_serv.c part10fork.h part10fork.c)

add_executable(part4 part4.h part4.c overal_functions.c overal_functions.h)

add_executable(part5 part5.h part5.c overal_functions.c overal_functions.h)

add_executable(part6 part6.h part6.c overal_functions.c overal_functions.h)

add_executable(part7 part7.h part7.c overal_functions.c overal_functions.h)

add_executable(part8serv part8_serv_uds.c part8_serv_uds.h overal_functions.c
overal_functions.h)

add_executable(part8client part8_client_uds.c part8_client_uds.h
overal_functions.c overal_functions.h)

add_executable(part9serv part9_sig_serv.c part9_sig_serv.c
overal_functions.c overal_functions.h)

add_executable(part10fork part10fork.c part10fork.h overal_functions.h
overal_functions.c)