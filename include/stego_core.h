#ifndef STEGO_CORE_H
#define STEGO_CORE_H
// stego_core.h — Common helper code
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

/* Print error and exit */
static inline void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(EXIT_FAILURE);
}

/* 8-byte magic so we know it is our payload */
static const uint8_t MAGIC_PNG[8] = { 'S','T','E','G','1','P','N','G' };

/* Write uint64 (little-endian) into 8 bytes */
static inline void write_u64_le(uint8_t out[8], uint64_t v) {
    for (int i = 0; i < 8; i++) out[i] = (uint8_t)((v >> (8*i)) & 0xFF);
}

/* Read uint64 (little-endian) from 8 bytes */
static inline uint64_t read_u64_le(const uint8_t in[8]) {
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) v |= ((uint64_t)in[i]) << (8*i);
    return v;
}

/* Get one bit from data (MSB first in each byte) */
static inline uint8_t get_bit(const uint8_t *data, uint64_t bit_index) {
    uint64_t byte_index = bit_index >> 3;
    int bit_in_byte = 7 - (int)(bit_index & 7);
    return (data[byte_index] >> bit_in_byte) & 1;
}

/* Make payload: MAGIC(8) + LEN(8) + MSG(len) */
static inline uint8_t *build_payload(const uint8_t magic[8],
                                     const uint8_t *msg,
                                     uint64_t msg_len,
                                     uint64_t *out_bytes) {
    if (!out_bytes) return NULL;
    if (msg_len > SIZE_MAX - 16) die("Message too big for memory.");
    uint64_t total = 16 + msg_len;
    uint8_t *buf = (uint8_t*)malloc((size_t)total);
    if (!buf) die("Out of memory for payload.");
    memcpy(buf, magic, 8);
    write_u64_le(buf + 8, msg_len);
    if (msg_len > 0) memcpy(buf + 16, msg, (size_t)msg_len);
    *out_bytes = total;
    return buf;
}

/* Read and check MAGIC, then get LEN */
static inline void parse_payload_prefix(const uint8_t *pfx,
                                        const uint8_t magic[8],
                                        uint64_t *out_len) {
    if (memcmp(pfx, magic, 8) != 0) die("No valid payload (magic mismatch).");
    *out_len = read_u64_le(pfx + 8);
}

#endif /* STEGO_CORE_H */
