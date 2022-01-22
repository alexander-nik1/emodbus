
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <emodbus/client/client.h>
#include <emodbus/protocols/tcp.h>
#include <emodbus/protocols/rtu.h>
#include <emodbus/protocols/ascii.h>
#include <emodbus/impl/posix/tcp-client.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/common.h>

static emb_tcp_client_t tcp_client =
{
    .transmit_timeout_ms = 100,
    .receive_timeout_ms =  100,
    .connect_timeout_ms = 3000,
    .first_reconnect_delay_ms = 0,
    .next_reconnects_delay_ms = 1000,
    .force_reconnect_delay_s = 0,
    .rxtx_timeouts_to_reconnect = 10,
    .flags = EMB_TCP_CLI_FORCE_RECONN_AT_RECV
             | EMB_TCP_CLI_FORCE_RECONN_AT_SEND
             | EMB_TCP_CLI_RECONNECT_AT_TIMEOUTS_COUNTER
             | EMB_TCP_CLI_NO_DELAY_WHILE_CONNECT
};

static uint8_t buf[256+16];

static uint8_t rx_buf[MAX_PDU_DATA_SIZE];
static uint8_t tx_buf[MAX_PDU_DATA_SIZE];

static emb_adu_t req_adu = {
    .transaction_id = 0,
    .server_id = 1,
    .flags = 0,
    .pdu = {
        .data = rx_buf,
        .max_size = MAX_PDU_DATA_SIZE
    }
};

static emb_adu_t ans_adu = {
    .pdu = {
        .data = tx_buf,
        .max_size = MAX_PDU_DATA_SIZE
    }
};

static uint16_t trans_is_counter = 0;

static int client_send_adu(emb_sync_client_t* _cli, emb_adu_t* _adu)
{
    (void)_cli;
    int r;

    trans_is_counter++;

    _adu->transaction_id = trans_is_counter;

    r = emb_tcp_encode_packet(_adu, buf, sizeof(buf));
    if(r < 0) {
//        fprintf(stderr, "Error with emb_tcp_encode_packet() :%d\n", r);
        return r;
    }

    r = emb_tcp_client_send(&tcp_client, buf, (unsigned int)r);
    if(r < 0) {
//        fprintf(stderr, "Error with emb_tcp_client_send(): %d\n", r);
        return r;
    }
    return modbus_success;
}

static int client_recv_adu(emb_sync_client_t* _cli, emb_adu_t* _adu)
{
    (void)_cli;
    int r;

    do
    {

        r = emb_tcp_client_recv_tcp(&tcp_client, buf, sizeof(buf));
        if(r < 0) {
            fprintf(stderr, "Error with emb_tcp_client_recv(): %s\n", emb_strerror(-r));
            return r;
        }

        r = emb_tcp_decode_packet(buf, (unsigned int)r, _adu);
        if(r != 0) {
            fprintf(stderr, "Error with emb_tcp_decode_packet(): %d\n", r);
        }
    }
    while(_adu->transaction_id != trans_is_counter);

    return r;
}

static emb_sync_client_t client =
{
    .req_adu = &req_adu,
    .ans_adu = &ans_adu,
    .send_adu = client_send_adu,
    .recv_adu = client_recv_adu,
    .n_retries = 0
};

#define msleep(_ms_)    usleep((_ms_)*1000)

#define poll_time       300

static long get_time_period_ms_from(const struct timeval* _from)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((now.tv_sec - _from->tv_sec) * 1000 + (now.tv_usec - _from->tv_usec)/1000);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    int res;
    int i;

    struct timeval start_time;

    uint16_t regs[65536];

    printf("Client test\n");

    res = emb_tcp_client_set_connection_options(&tcp_client, "192.168.1.221", 8502);
    if(res != 0)
        return -1;

    res = emb_tcp_client_init(&tcp_client);
    if(res != 0)
        return -1;

    for(i=0; i<1000; ++i) {

        gettimeofday(&start_time, NULL);

        res = emb_sync_client_read_regs(&client, 1, EMB_RR_INPUTS, 0x0000, EMB_ARR_SIZE(regs), regs);

        printf("%d: reconnects:%d state:%s good:%d bad:%d %s(%d)\n",
               i,
               tcp_client.connection_attempts,
               tcp_client.state == emb_tcs_connected ? "connected" : "disconnected",
               client.good_transactions,
               client.bad_transactions,
               i == modbus_success ? "ok" : emb_strerror(-res),
               res);


        long period = poll_time - get_time_period_ms_from(&start_time);

        if (period > 0) {
            printf("Polling time: %ld\n", get_time_period_ms_from(&start_time));
            msleep((unsigned int)period);
        }
        else {
            printf("==========================================> Cycle time overflow!\n");
        }

        fflush(stdout);
    }

    emb_tcp_client_deinit(&tcp_client);

    return 0;
}
