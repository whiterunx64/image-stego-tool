#define _POSIX_C_SOURCE 200809L

/*
 * lsb_encode.c — PNG encoder (LSB in RGB). Alpha is not changed.
 */

#include <string.h>
#include "stego_core.h"
#include "png_io.h"
#include "png_stego.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s -i <input.png> -o <output.png> -m <message>\n"
        "\n"
        "Notes:\n"
        "  - Data is stored in RGB LSBs. Alpha is not changed.\n"
        "  - Max size: ~ (width*height*3)/8 bytes minus 16 header bytes.\n",
        prog
    );
}

static void cmd_encode(const char *in_path, const char *out_path, const char *message) {
    if (!in_path || !out_path || !message) die("Need -i <input> -o <output> -m <message>");

    png_image_t img;
    png_read_all(in_path, &img);

    uint64_t msg_len = (uint64_t)strlen(message);
    uint64_t payload_size = 0;
    uint8_t *payload = build_payload(MAGIC_PNG, (const uint8_t*)message, msg_len, &payload_size);

    uint64_t cap_bits = png_capacity_bits(&img);
    uint64_t need_bits = payload_size * 8ULL;
    if (need_bits > cap_bits) {
        free(payload);
        png_free_image(&img);
        die("Message too big. Capacity: %llu bits (~%llu bytes). Need: %llu bits (~%llu bytes).",
            (unsigned long long)cap_bits, (unsigned long long)(cap_bits/8),
            (unsigned long long)need_bits, (unsigned long long)(need_bits/8));
    }

    fprintf(stderr, "PNG capacity: %llu bits (~%llu bytes). Embedding %llu bytes.\n",
            (unsigned long long)cap_bits, (unsigned long long)(cap_bits/8),
            (unsigned long long)msg_len);

    png_embed_payload(&img, payload, payload_size);
    png_write_all(out_path, &img);

    free(payload);
    png_free_image(&img);

    fprintf(stderr, "Done. Wrote %s\n", out_path);
}

int main(int argc, char **argv) {
    const char *in_path = NULL;
    const char *out_path = NULL;
    const char *message = NULL;

    if (argc < 2) { usage(argv[0]); return 1; }

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) && i+1 < argc) in_path = argv[++i];
        else if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i+1 < argc) out_path = argv[++i];
        else if ((strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0) && i+1 < argc) message = argv[++i];
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { usage(argv[0]); return 0; }
        else die("Unknown or incomplete option: %s", argv[i]);
    }

    cmd_encode(in_path, out_path, message);
    return 0;
}
