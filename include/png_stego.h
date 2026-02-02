#ifndef PNG_STEGO_H
#define PNG_STEGO_H
#include <stdint.h>
#include <stdlib.h>
#include "stego_core.h"
#include "png_io.h"

/* Capacity: 3 bits per pixel (R,G,B) */
static inline uint64_t png_capacity_bits(const png_image_t *img) {
    uint64_t pixels = (uint64_t)img->width * (uint64_t)img->height;
    return pixels * 3ULL;
}

/* Iterator to read LSBs from R,G,B in scanline order */
typedef struct {
    const png_image_t *img;
    uint32_t y, x;
    int ch; /* 0=R,1=G,2=B */
} png_lsb_iter_t;

static inline void png_lsb_iter_init(png_lsb_iter_t *it, const png_image_t *img) {
    it->img = img; it->y = 0; it->x = 0; it->ch = 0;
}

static inline int png_lsb_iter_next(png_lsb_iter_t *it) {
    while (it->y < it->img->height) {
        const uint8_t *row = it->img->row_ptrs[it->y];
        while (it->x < it->img->width) {
            while (it->ch < 3) {
                int bit = row[it->x * it->img->channels + it->ch] & 1;
                it->ch++;
                return bit;
            }
            it->ch = 0;
            it->x++;
        }
        it->x = 0;
        it->y++;
    }
    return -1; /* no more bits */
}

/* Hide payload bits into R,G,B LSBs */
static inline void png_embed_payload(png_image_t *img,
                                     const uint8_t *payload,
                                     uint64_t payload_bytes) {
    uint64_t need_bits = payload_bytes * 8ULL;
    uint64_t cap_bits  = png_capacity_bits(img);
    if (need_bits > cap_bits) die("Message too big for this PNG.");

    uint64_t bit_index = 0;
    for (uint32_t y = 0; y < img->height && bit_index < need_bits; y++) {
        uint8_t *row = img->row_ptrs[y];
        for (uint32_t x = 0; x < img->width && bit_index < need_bits; x++) {
            uint8_t *px = &row[x * img->channels];
            if (bit_index < need_bits) { px[0] = (uint8_t)((px[0] & 0xFE) | get_bit(payload, bit_index++)); } /* R */
            if (bit_index < need_bits) { px[1] = (uint8_t)((px[1] & 0xFE) | get_bit(payload, bit_index++)); } /* G */
            if (bit_index < need_bits) { px[2] = (uint8_t)((px[2] & 0xFE) | get_bit(payload, bit_index++)); } /* B */
        }
    }
}

/* Read payload and print message to stdout */
static inline void png_extract_payload(const png_image_t *img, const uint8_t magic[8]) {
    uint64_t cap_bits = png_capacity_bits(img);
    if (cap_bits < 128) die("Not enough capacity for header.");

    png_lsb_iter_t it;
    png_lsb_iter_init(&it, img);

    /* Read first 16 bytes: MAGIC + LEN */
    uint8_t header[16] = {0};
    for (int i = 0; i < 128; i++) {
        int b = png_lsb_iter_next(&it);
        if (b < 0) die("Unexpected end while reading header.");
        header[i >> 3] = (uint8_t)((header[i >> 3] << 1) | (uint8_t)b);
    }

    uint64_t msg_len = 0;
    parse_payload_prefix(header, magic, &msg_len);

    uint64_t msg_bits = msg_len * 8ULL;
    if (128 + msg_bits > cap_bits) die("Claimed message does not fit in image.");

    uint8_t *msg = NULL;
    if (msg_len > 0) {
        if (msg_len > SIZE_MAX) die("Message too big for memory.");
        msg = (uint8_t*)calloc((size_t)msg_len + 1, 1);
        if (!msg) die("Out of memory for message.");
    }

    for (uint64_t i = 0; i < msg_bits; i++) {
        int b = png_lsb_iter_next(&it);
        if (b < 0) die("Unexpected end while reading message.");
        msg[i >> 3] = (uint8_t)((msg[i >> 3] << 1) | (uint8_t)b);
    }

    if (msg_len > 0) {
        msg[msg_len] = '\0';
        printf("%s\n", (char*)msg);
        free(msg);
    } else {
        printf("\n");
    }
}

#endif /* PNG_STEGO_H */
