
#include <emodbus/base/common.h>

#include <stdint.h>

#if EMODBUS_PACKETS_DUMPING

FILE* emb_debug_output;

/**
 * @brief Print packet contents
 *
 * Function will print to given file descriptor
 * data, pointed by _pkt.
 *
 * @param [in] _f File descriptor to write to. (if is zero, then nothing will be printed)
 * @param [in] _prefix Some prefix to print it before data.
 * @param [in] _pkt The packet.
 * @param [in] _size Size of packet in bytes.
 */
void dbg_print_packet(const char* _prefix, const void* _pkt, unsigned int _size) {
    if(emb_debug_output) {
        int i;
        fprintf(emb_debug_output, "%s", _prefix);
        for(i=0; i<_size; ++i) {
            fprintf(emb_debug_output, "%02X ", ((uint8_t*)_pkt)[i]);
        }
        fprintf(emb_debug_output, "\n");
        fflush(emb_debug_output);
    }
}

#endif // EMODBUS_PACKETS_DUMPING
