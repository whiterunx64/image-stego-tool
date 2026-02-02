#define _POSIX_C_SOURCE 200809L
/* Usage
 * ./lsb-decode -i stego.png
 */
#include <string.h>
#include "stego_core.h"
#include "png_io.h"
#include "png_stego.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s -i <input.png>\n"
        "\n"
        "Notes:\n"
        "  - Input must be a PNG made by the encoder.\n",
        prog
    );
}

static void cmd_decode(const char *in_path) {
    if (!in_path) die("Need -i <input.png>");

    png_image_t img;
    png_read_all(in_path, &img);
    png_extract_payload(&img, MAGIC_PNG);
    png_free_image(&img);
}

int main(int argc, char **argv) {
    const char *in_path = NULL;

    if (argc < 2) { usage(argv[0]); return 1; }

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) && i+1 < argc) in_path = argv[++i];
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { usage(argv[0]); return 0; }
        else die("Unknown or incomplete option: %s", argv[i]);
    }

    cmd_decode(in_path);
    return 0;
}
