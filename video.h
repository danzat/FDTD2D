#ifndef VIDEO_H
#define VIDEO_H

#include <cairo.h>
#include <theora/theoraenc.h>
#include <ogg/ogg.h>

#include <stdio.h>

typedef enum {
    VIDEO_OK,
    VIDEO_MEMORY_ALLOCATION_ERROR,
    VIDEO_CAIRO_ERROR,
    VIDEO_THEORA_ERROR,
    VIDEO_NULL_POINTER_REFERENCE,
    VIDEO_IO_ERROR,
    VIDEO_IMAGE_SIZE_MISMATCH
} VIDEO_ret;

typedef struct {
    th_info          ti;
    th_enc_ctx       *tx;
    ogg_stream_state oss;
} video_state_t;

VIDEO_ret VIDEO_allocate_ycbcr_buffer(th_ycbcr_buffer ycbcr, cairo_surface_t *cs);
void VIDEO_free_ycbcr_buffer(th_ycbcr_buffer ycbcr);
VIDEO_ret VIDEO_rgb2ycbcr(cairo_surface_t *cs, th_ycbcr_buffer ycbcr);

VIDEO_ret VIDEO_init(video_state_t *vs, int width, int height);
VIDEO_ret VIDEO_write_frame(video_state_t *vs, th_ycbcr_buffer ycbcr, int last, FILE *fp);
VIDEO_ret VIDEO_write_pages(video_state_t *vs, FILE *fp);

#endif /* VIDEO_H */
