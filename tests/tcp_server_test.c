
#include <emodbus/impl/posix/tcp-server.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    emb_tcp_server_t tcp_server;
    uint8_t buffer[256];

    tcp_server.rx_timeout_ms = 1000;

    if(emb_tcp_server_init(&tcp_server, htonl(INADDR_ANY), 2020)) {
        fprintf(stderr, "Error with tcp_server_init() : %m\n");
    }

    for (;;) {
        int client_id;
        const int r = emb_tcp_server_receive(&tcp_server, &client_id, buffer, sizeof(buffer));
        if(r > 0) {
            emb_tcp_server_send(&tcp_server, client_id, buffer, (unsigned int)r);
        }
    }

    emb_tcp_server_deinit(&tcp_server);

//    return 0;
}
