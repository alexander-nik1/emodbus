
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "emodbus/base/common.h"
#include "emodbus/base/modbus_errno.h"
#include "emodbus/client/client.h"
#include "emodbus/protocols/ascii.h"
#include "emodbus/impl/posix/serial_port.h"
#include "emodbus/base/bit_array.h"

// =============================================================================================
// Serial port part

#define TTY_NAME "/dev/ttyUSB0"
#define TTY_BAUD 1500000

static emb_serial_port_t serial_port =
{
    .tty_name = TTY_NAME,
    .baudrate = TTY_BAUD,
    .timeout_ms = 100,
    .final_delay_ms = 5
};

// =============================================================================================
// Client part

static uint8_t buf[256 * 2 + 16];

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

static int client_send_adu(emb_sync_client_t* _cli, const emb_adu_t* _adu);
static int client_recv_adu(emb_sync_client_t* _cli, emb_adu_t* _adu);

static emb_sync_client_t client =
{
    .req_adu = &req_adu,
    .ans_adu = &ans_adu,
    .send_adu = client_send_adu,
    .recv_adu = client_recv_adu
};

static int client_send_adu(emb_sync_client_t* _cli, const emb_adu_t* _adu)
{
    (void)_cli;
    int r;

    r = emb_ascii_encode_packet(_adu, buf, sizeof(buf));
    if(r < 0) {
        fprintf(stderr, "Error with emb_ascii_encode_packet() :%d\n", r);
        return r;
    }

    r = emb_serial_port_send(&serial_port, buf, (unsigned int)r);
    if(r < 0) {
        fprintf(stderr, "Error with emb_serial_port_send(): %d\n", r);
        return r;
    }
    return modbus_success;
}

static int client_recv_adu(emb_sync_client_t* _cli, emb_adu_t* _adu)
{
    (void)_cli;
    int r;

    r = emb_serial_port_receive_ascii(&serial_port, buf, sizeof(buf));
    if(r < 0) {
        fprintf(stderr, "Error with emb_serial_port_receive_ascii(): %s\n", emb_strerror(-r));
        return r;
    }

    r = emb_ascii_decode_packet(buf, (unsigned int)r, _adu);
    if(r != 0) {
        fprintf(stderr, "Error with emb_ascii_decode_packet(): %d\n", r);
    }
    return r;
}

int main()
{
    int res;
    int i;
    memset(buf, 0, sizeof(buf));

    emb_sync_client_init(&client);

    printf("Connecting to '%s' baud: %d\n", TTY_NAME, TTY_BAUD);

    emb_serial_port_init(&serial_port);
    if(emb_serial_port_open(&serial_port) != 0) {
        fprintf(stderr, "Error: serial_port_open() : %m\n");
        return -1;
    }

    uint16_t regs[65536];

    for(i=0; i<1000; ++i) {
        res = emb_sync_client_read_regs(&client, 1, EMB_RR_HOLDINGS, 0x0000, EMB_ARR_SIZE(regs), regs);
        fputs("\r", stdout);
        printf("%d:  good:%d bad:%d %s",
               i,
               client.good_transactions,
               client.bad_transactions,
               i == modbus_success ? "ok" : emb_strerror(-res));
        fputs("                             \n", stdout);
        fflush(stdout);
        usleep(1000*1);
    }
}

