#include "video.h"

#include <cairo.h>
#include <theora/theoraenc.h>
#include <ogg/ogg.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* 
 * Assumptions made:
 *  1. The chroma format is 444, meaning Y, Cb and Cr planes are the same size.
 *  2. Input image is 24bit RGB
 */

VIDEO_ret VIDEO_allocate_ycbcr_buffer(th_ycbcr_buffer ycbcr, cairo_surface_t *cs)
{
    int w = 0, h = 0;
    int fixed_w = 0, fixed_h = 0;
    if (NULL == cs) {
        fprintf(stderr, "ERROR<VIDEO>: NULL cairo surface\n");
        return VIDEO_NULL_POINTER_REFERENCE;
    }
    w = cairo_image_surface_get_width(cs);
    h = cairo_image_surface_get_height(cs);
    /* the Y'CbCr frames' dimensions should be multiples of 16: */
    fixed_w = (w + 15) & ~15;
    fixed_h = (h + 15) & ~15;
    /* As I've mentioned before - the chroma format is 444, so all planes are
     * the same size */
    ycbcr[0].width  = fixed_w;
    ycbcr[0].height = fixed_h;
    ycbcr[0].stride = fixed_w;
    ycbcr[1].width  = fixed_w;
    ycbcr[1].height = fixed_h;
    ycbcr[1].stride = fixed_w;
    ycbcr[2].width  = fixed_w;
    ycbcr[2].height = fixed_h;
    ycbcr[2].stride = fixed_w;
    ycbcr[0].data = malloc(ycbcr[0].stride * ycbcr[0].height);
    ycbcr[1].data = malloc(ycbcr[1].stride * ycbcr[1].height);
    ycbcr[2].data = malloc(ycbcr[2].stride * ycbcr[2].height);
    if ((NULL == ycbcr[0].data) || (NULL == ycbcr[1].data) || (NULL == ycbcr[2].data)) {
        fprintf(stderr, "ERROR<VIDEO>: Could not allocate memory for Y\'CbCr data\n");
        return VIDEO_MEMORY_ALLOCATION_ERROR;
    }
    return VIDEO_OK;
}

unsigned char clamp(long c)
{
    if (c <= 0) return 0;
    if (c >= 255) return 255;
    return c;
}

VIDEO_ret VIDEO_rgb2ycbcr(cairo_surface_t *cs, th_ycbcr_buffer ycbcr)
{
    long i = 0, j = 0;
    unsigned char *rgb = NULL;
    unsigned char *y = NULL, *cb = NULL, *cr = NULL;
    unsigned char r = 0, g = 0, b = 0;
    int w = 0, h = 0, s = 0;
    int y_w = 0, y_h = 0;
    unsigned long color;
    if (NULL == cs) {
        fprintf(stderr, "ERROR<VIDEO>: NULL cairo surface\n");
        return VIDEO_NULL_POINTER_REFERENCE;
    }
    if ((NULL == ycbcr[0].data) || (NULL == ycbcr[1].data) || (NULL == ycbcr[2].data)) {
        fprintf(stderr, "ERROR<VIDEO>: NULL Y\'CbCr data\n");
        return VIDEO_NULL_POINTER_REFERENCE;
    }
    y_w = ycbcr[0].width;
    y_h = ycbcr[0].height;
    w = cairo_image_surface_get_width(cs);
    s = cairo_image_surface_get_stride(cs);
    h = cairo_image_surface_get_height(cs);
    if ((y_h < h) || (y_w < w)) {
        fprintf(stderr, "ERROR<VIDEO>: Y\'CbCr buffer must be at least as large as the input image\n");
        return VIDEO_IMAGE_SIZE_MISMATCH;
    }
    rgb = cairo_image_surface_get_data(cs);
    y  = ycbcr[0].data;
    cb = ycbcr[1].data;
    cr = ycbcr[2].data;

    for (i = 0; i < w; ++i) {
        for (j = 0; j < h; ++j) {
            color = ((unsigned long *)(rgb + s * j))[i];
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = color & 0xFF;

            y[i + j*y_w]  = 16  + (( 66  * r  + 129 * g + 25  * b + 128) >> 8);
            cb[i + j*y_w] = 128 + ((-38  * r  - 74  * g + 112 * b + 128) >> 8);
            cr[i + j*y_w] = 128 + (( 112 * r  - 94  * g - 18  * b + 128) >> 8);
        }
    }
    return VIDEO_OK;
}

VIDEO_ret VIDEO_init(video_state_t *vs, int width, int height)
{
    th_comment tc;
    ogg_packet op;
    int ret;
    if (NULL == vs) {
        fprintf(stderr, "ERROR<VIDEO>: NULL video_state pointer\n");
        return VIDEO_NULL_POINTER_REFERENCE;
    }

    srand(time(NULL));
    if (ogg_stream_init(&vs->oss, rand()) != 0) {
        fprintf(stderr, "ERROR<VIDEO>: Could not open OGG stream state\n");
        return VIDEO_THEORA_ERROR;
    }
    
    vs->ti.frame_width = (width + 15) & ~15;
    vs->ti.frame_height = (height + 15) & ~15;
    vs->ti.pic_width = width;
    vs->ti.pic_height = height;
    vs->ti.pic_x = 0;
    vs->ti.pic_y = 0;
    vs->ti.fps_numerator = 24;
    vs->ti.fps_denominator = 1;
    vs->ti.aspect_numerator = 0;
    vs->ti.aspect_denominator = 0;
    vs->ti.colorspace = TH_CS_UNSPECIFIED;
    vs->ti.pixel_fmt = TH_PF_444;
    vs->ti.target_bitrate= 0;
    vs->ti.quality = 63;
    /* In one-pass mode, the key-frame frequency is 64, so the shift value is
     * log2(63) = 5 */
    vs->ti.keyframe_granule_shift = 5;
    vs->tx = th_encode_alloc(&vs->ti);

    /* First packet in the Theora header */
    th_comment_init(&tc);
    ret = th_encode_flushheader(vs->tx, &tc, &op);
    if (ret <= 0) {
        fprintf(stderr, "ERROR<VIDEO>: Internal Theora error\n");
        return VIDEO_THEORA_ERROR;
    }
    ogg_stream_packetin(&vs->oss, &op);
    /* Rest of the Theora headers */
    th_comment_clear(&tc);
    while (1) {
        ret = th_encode_flushheader(vs->tx, &tc, &op);
        if (ret < 0) {
            fprintf(stderr, "ERROR<VIDEO>: Internal Theora error\n");
            return VIDEO_THEORA_ERROR;
        } else if (0 == ret) {
            break;
        }
        ogg_stream_packetin(&vs->oss, &op);
    }
    return VIDEO_OK;
}

VIDEO_ret VIDEO_write_pages(video_state_t *vs, FILE *fp)
{
    size_t ret;
    ogg_page og;
    if (NULL == vs) {
        fprintf(stderr, "ERROR<VIDEO>: NULL video_state pointer\n");
        return VIDEO_NULL_POINTER_REFERENCE;
    }
    while (ogg_stream_pageout(&vs->oss, &og) != 0) {
        ret = fwrite(og.header, og.header_len, 1, fp);
        if (ret == 0) {
            fprintf(stderr, "ERROR<VIDEO>: Could not write ogg header\n");
            return VIDEO_IO_ERROR;
        }
        ret = fwrite(og.body, og.body_len, 1, fp);
        if (ret == 0) {
            fprintf(stderr, "ERROR<VIDEO>: Could not write ogg body\n");
            return VIDEO_IO_ERROR;
        }
    }
    return VIDEO_OK;
}

VIDEO_ret VIDEO_write_frame(video_state_t *vs, th_ycbcr_buffer ycbcr, int last, FILE *fp)
{
    int ret = 0;
    ogg_packet op;
    ret = th_encode_ycbcr_in(vs->tx, ycbcr);
    switch (ret) {
        case TH_EFAULT:
            fprintf(stderr, "ERROR<VIDEO>: Either the context or the buffer are null\n");
            return VIDEO_THEORA_ERROR;
        case TH_EINVAL:
            fprintf(stderr, "ERROR<VIDEO>: Buffer size does not match the frame or the picture\n");
            return VIDEO_THEORA_ERROR;
        case 0:
            break;
        default:
            fprintf(stderr, "ERROR<VIDEO>: Could not encode frame\n");
            return VIDEO_THEORA_ERROR;
    }
    ret = th_encode_packetout(vs->tx, last, &op);
    if (0 == ret) {
        fprintf(stderr, "ERROR<VIDEO>: Could not produce packet\n");
        return VIDEO_THEORA_ERROR;
    }
    ogg_stream_packetin(&vs->oss, &op);
    ret = VIDEO_write_pages(vs, fp);
    return ret;
}
