#ifndef PNG_IO_H
#define PNG_IO_H
#include <stdint.h>
#include <string.h>
#include <png.h>
#include <setjmp.h>
#include "stego_core.h"

/* PNG image container */
typedef struct {
    uint32_t width, height;
    int color_type, bit_depth;
    png_bytep *row_ptrs;
    size_t rowbytes;
    int channels; /* 3 for RGB, 4 for RGBA */
} png_image_t;

/* Read PNG and convert to RGB/RGBA (8-bit) */
static inline void png_read_all(const char *path, png_image_t *out) {
    memset(out, 0, sizeof(*out));
    FILE *fp = fopen(path, "rb");
    if (!fp) die("Cannot open PNG: %s", path);

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) die("png_create_read_struct failed.");
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) die("png_create_info_struct failed.");

    if (setjmp(png_jmpbuf(png_ptr))) die("Error while reading PNG.");

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    out->width  = png_get_image_width(png_ptr, info_ptr);
    out->height = png_get_image_height(png_ptr, info_ptr);
    out->color_type = png_get_color_type(png_ptr, info_ptr);
    out->bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

    /* Convert to 8-bit RGB(A) */
    if (out->bit_depth == 16) png_set_strip_16(png_ptr);
    if (out->color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
    if (out->color_type == PNG_COLOR_TYPE_GRAY && out->bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    if (out->color_type == PNG_COLOR_TYPE_GRAY || out->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    out->rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    out->channels = png_get_channels(png_ptr, info_ptr);
    if (out->channels < 3) die("PNG must be RGB or RGBA after convert.");

    out->row_ptrs = (png_bytep*)malloc(sizeof(png_bytep) * out->height);
    if (!out->row_ptrs) die("Out of memory for rows.");

    uint8_t *img = (uint8_t*)malloc(out->rowbytes * out->height);
    if (!img) die("Out of memory for image.");
    for (uint32_t y = 0; y < out->height; y++) out->row_ptrs[y] = img + y * out->rowbytes;

    png_read_image(png_ptr, out->row_ptrs);
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
}

/* Write PNG from memory. Keep current channels. */
static inline void png_write_all(const char *path, const png_image_t *img) {
    FILE *fp = fopen(path, "wb");
    if (!fp) die("Cannot open output PNG: %s", path);

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) die("png_create_write_struct failed.");
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) die("png_create_info_struct failed.");

    if (setjmp(png_jmpbuf(png_ptr))) die("Error while writing PNG.");

    png_init_io(png_ptr, fp);

    int color_type = (img->channels == 4) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
    png_set_IHDR(png_ptr, info_ptr,
                 img->width, img->height,
                 8, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    png_write_image(png_ptr, img->row_ptrs);
    png_write_end(png_ptr, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

/* Free image memory */
static inline void png_free_image(png_image_t *img) {
    if (img->row_ptrs) {
        if (img->row_ptrs[0]) free(img->row_ptrs[0]);
        free(img->row_ptrs);
    }
    memset(img, 0, sizeof(*img));
}

#endif /* PNG_IO_H */
