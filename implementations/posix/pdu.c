
#include <emodbus/impl/posix/pdu.h>
#include <stdlib.h>
#include <errno.h>

int emb_posix_alloc_pdu_data(emb_pdu_t* _pdu)
{
    if(_pdu) {
        if((_pdu->data = malloc(MAX_PDU_DATA_SIZE))) {
            _pdu->max_size = MAX_PDU_DATA_SIZE;
            return 0;
        }
        else {
            return -ENOMEM;
        }
    }
    return -EINVAL;
}

int emb_posix_free_pdu_data(emb_pdu_t* _pdu)
{
    if(_pdu) {
        if(_pdu->data) {
            free(_pdu->data);
            _pdu->data = NULL;
            _pdu->max_size = 0;
            return 0;
        }
    }
    return -EINVAL;
}

