
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include <emodbus/client/client.h>
#include <emodbus/protocols/tcp.h>
#include <emodbus/protocols/rtu.h>
#include <emodbus/protocols/ascii.h>
#include <emodbus/impl/posix/tcp-client.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/common.h>
#include <emodbus/base/bit_array.h>

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
        //     | EMB_TCP_CLI_NO_DELAY_WHILE_CONNECT
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

void print_adu(FILE* _f, const emb_adu_t* _adu)
{
    uint8_t i;
    fprintf(_f, "srv:0x%02X f:0x%02X data:", _adu->server_id, _adu->pdu.function);
    for(i=0; i<_adu->pdu.data_size; ++i) {
        fprintf(_f, "%02X ", ((uint8_t*)_adu->pdu.data)[i]);
    }
    fprintf(_f, "\n");
}

static int client_send_adu(emb_sync_client_t* _cli, emb_adu_t* _adu)
{
    (void)_cli;
    int r;

    trans_is_counter++;

    _adu->transaction_id = trans_is_counter;

    //printf("<<");
    //print_adu(stdout, _adu);

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

        //printf(">>");
        //print_adu(stdout, _adu);
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

void coils_test();

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    int res;
    int i;

    uint8_t coils[65536/8];

    printf("Client test\n");

    res = emb_tcp_client_set_connection_options(&tcp_client, "127.0.0.1", 8502);
    if(res != 0)
        return -1;

    res = emb_tcp_client_init(&tcp_client);
    if(res != 0)
        return -1;

    coils_test();

    emb_tcp_client_deinit(&tcp_client);

    return 0;
}

void randomize_mem(void* _bytes, size_t _size)
{
    size_t i;
    for(i=0; i<_size; ++i) {
        ((uint8_t*)_bytes)[i] = (uint8_t)rand();
    }
}

int compare_bits(const void* _b1, const void* _b2, size_t _size)
{
    size_t i;
    int errors=0;
    for(i=0; i<_size; ++i) {
        if(((const uint8_t*)_b1)[i] != ((const uint8_t*)_b2)[i]) {
            printf("bits %d-%d is differ (0x%02X != 0x%02X)!\n", i*8, (i+1)*8, ((const uint8_t*)_b1)[i], ((const uint8_t*)_b2)[i]);
            errors++;
        }
    }
    return errors;
}

void coils_test()
{
    int i;
    int res;
    int errors = 0;

    uint8_t coils1[65536/8];
    uint8_t coils2[65536/8];

    memset(coils1, 0, sizeof(coils1));
    memset(coils2, 0, sizeof(coils2));

    srand(time(0));

    for (i=0; i<10000; ++i)
    {
        uint16_t start = (uint16_t)rand() % 1000;
        uint16_t quantity = (uint16_t)rand() % 2000 + 30000;

        randomize_mem(coils1, sizeof(coils1));
        randomize_mem(coils2, sizeof(coils2));

        res = emb_sync_client_write_coils(&client, 1, start, quantity, coils1);
        if(res != modbus_success)
            printf("Error while emb_sync_client_write_coils():%s\n", emb_strerror(-res));

        res = emb_sync_client_read_bits(&client, 1, EMB_RB_DISCRETE_INPUTS, start, quantity, coils2);
        if(res != modbus_success)
            printf("Error while emb_sync_client_read_bits():%s\n", emb_strerror(-res));

        res = emb_bit_arr_cmp(coils1, coils2, quantity);
        if (res != 0)
            errors++;

        printf("Test# %d Errors: %d\n", i, res);

        //usleep(1000*1);
    }

    printf("All errors: %d\n", errors);
}


