
#include <emodbus/impl/posix/tcp-server.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    tcp_server_t tcp_server;
    uint8_t buffer[256];

    if(tcp_server_init(&tcp_server, htonl(INADDR_ANY), 2020)) {
        fprintf(stderr, "Error with tcp_server_init() : %m\n");
    }

    for (;;) {
        int client_id;
        const int r = tcp_server_receive(&tcp_server, &client_id, buffer, sizeof(buffer), 1000);
        if(r > 0) {
            tcp_server_send(&tcp_server, client_id, buffer, (unsigned int)r);
        }
    }

    tcp_server_deinit(&tcp_server);

//    return 0;
}
