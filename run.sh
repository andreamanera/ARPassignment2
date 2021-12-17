gcc master.c -o master
gcc named_pipe.c -o np
gcc unnamed_pipe.c -o up
gcc socket.c -o sck
gcc circular_buffer.c -o cb -lpthread -lrt
./master