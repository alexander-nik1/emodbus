
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "emodbus/base/modbus_errno.h"
#include "emodbus/client/client.h"
#include "emodbus/protocols/rtu.h"
#include "emodbus/impl/posix/serial_port.h"
#include "emodbus/impl/posix/tcp-server.h"

#include "emodbus/client/read_regs.h"
#include "emodbus/client/write_mask_reg.h"
#include "emodbus/client/read_write_regs.h"
#include "emodbus/client/write_single_reg.h"
#include "emodbus/client/write_multi_regs.h"

#define ARR_SIZE(_arr_)     (sizeof(_arr_)/sizeof(_arr_[0]))

// =============================================================================================
// RTU part

static struct serial_port_t serial_port =
{
    .tty_name = "/dev/ttyUSB2",
    .baudrate = 115200,
    .timeout_ms = 100,
    .override_final_delay_ms = 20
};


// =============================================================================================
// Client

static uint8_t buf[256+16];

static int client_send_adu(struct __emb_sync_client_t* _cli, const emb_adu_t* _adu)
{
    (void)_cli;
    int r;

    r = emb_rtu_encode_packet(_adu, buf, sizeof(buf));
    if(r < 0) {
        fprintf(stderr, "Error with emb_rtu_encode_packet() :%d\n", r);
        return r;
    }

    r = serial_port_send(&serial_port, buf, (unsigned int)r);
    if(r < 0) {
        fprintf(stderr, "Error with serial_port_send(): %d\n", r);
        return r;
    }
    return modbus_success;
}

static int client_recv_adu(struct __emb_sync_client_t* _cli, emb_adu_t* _adu)
{
    (void)_cli;
    int r;

    r = serial_port_receive(&serial_port, buf, sizeof(buf));
    if(r < 0) {
        fprintf(stderr, "Error with serial_port_receive(): %s\n", emb_strerror(-r));
        return r;
    }

    r = emb_rtu_decode_packet(buf, (unsigned int)r, _adu);
    if(r != 0) {
        fprintf(stderr, "Error with emb_rtu_decode_packet(): %d\n", r);
    }
    return r;
}

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

static emb_sync_client_t client =
{
    .req_adu = &req_adu,
    .ans_adu = &ans_adu,
    .send_adu = client_send_adu,
    .recv_adu = client_recv_adu
};

//

int main()
{
    int i, tmp;

    memset(buf, 0, sizeof(buf));

    serial_port_init(&serial_port);

    if(serial_port_open(&serial_port) != 0) {
        fprintf(stderr, "Error: serial_port_open() : %m\n");
        return -1;
    }

//    const uint16_t wr_data[] = { 12340, 12341, 12342, 12343, 12344 };

    uint16_t regsw[65536] = {0};
    uint16_t regsr[65536] = {0};

    uint32_t test_size = 65536;

    for(i=0; i<ARR_SIZE(regsw); ++i) {
        regsw[i] = rand();
    }

    tmp = emb_sync_client_write_regs(&client, 1, 0, test_size, regsw);
    if(tmp != modbus_success)
        printf("emb_sync_client_write_regs() = %d\n", tmp);

    for(i=0; i<1024; ++i) {
        uint16_t addr = (uint16_t)rand() % test_size;
        uint16_t and = (uint16_t)rand() % test_size;
        uint16_t or = (uint16_t)rand() % test_size;
        regsw[addr] = (uint16_t)rand() % test_size;
        tmp = emb_sync_client_write_reg(&client, 1, addr, regsw[addr]);
        if(tmp != modbus_success)
            printf("emb_sync_client_write_reg() = %d\n", tmp);

        addr = (uint16_t)rand() % test_size;
        regsw[addr] = (regsw[addr] & and) | (or & ~and);
        tmp = emb_sync_client_mask_reg(&client, 1, addr, and, or);
        if(tmp != modbus_success)
            printf("emb_sync_client_mask_reg() = %d\n", tmp);


        uint16_t wa = (uint16_t)rand() % (test_size-10);
        uint16_t ra = (uint16_t)rand() % (test_size-10);
        uint16_t wq = (uint16_t)rand() % (test_size - wa)+1;
        uint16_t rq = (uint16_t)rand() % (test_size - ra)+1;
        if(wq > 120)
            wq = 120;
        if(rq > 120)
            rq = 120;
        tmp = emb_sync_client_rdwr_regs(&client, 1, wa, wq, regsw + wa, ra, rq, regsr + ra);
        if(tmp != modbus_success)
            printf("emb_sync_client_rdwr_regs() = %d\n", tmp);
        else {
            int j;
            for(j=0; j<rq; ++j) {
                if(regsw[ra+j] != regsr[ra+j]) {
                    printf("Error: 0x%04X != 0x%04X (%d)\n", regsw[ra+j], regsr[ra+j], j);
                }
            }
        }
    }

    tmp = emb_sync_client_read_regs(&client, 1, EMB_RR_HOLDINGS, 0, test_size, regsr);
    if(tmp != modbus_success)
        printf("emb_sync_client_read_regs() = %d\n", tmp);

    int errors = 0;
    for(i=0; i<test_size; ++i) {
        if(regsw[i] != regsr[i]) {
            printf("Error: 0x%04X != 0x%04X (%d)\n", regsw[i], regsr[i], i);
            ++errors;
        }
    }

    tmp = emb_sync_client_read_regs(&client, 1, EMB_RR_INPUTS, 0, test_size, regsr);
    if(tmp != modbus_success)
        printf("emb_sync_client_read_regs() = %d\n", tmp);

    for(i=0; i<test_size; ++i) {
        if(regsw[i] != regsr[i]) {
            printf("Error: 0x%04X != 0x%04X (%d)\n", regsw[i], regsr[i], i);
            ++errors;
        }
    }

    printf("Errors: %d\n", errors);

    serial_port_close(&serial_port);
}
