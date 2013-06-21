#include "fdtd2d.h"
#include "assignment.h"
#include "video.h"

#include <math.h>
#include <cairo/cairo-svg.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#ifndef MAX_PATH
    #define MAX_PATH 255
#endif

const char *optstring = "ho:g:v:s:f:";
struct option long_options[] = {
    {"help",    0,  NULL, 'h'},
    {"output",  1,  NULL, 'o'},
    {"gamma",   1,  NULL, 'g'},
    {"video",   1,  NULL, 'v'},
    {"stills",  1,  NULL, 's'},
    {"func",    1,  NULL, 'f'},
    {NULL,      0,  NULL, 0}
};

extern const iterate_func_t iterate_funcs[];
extern const render_func_t render_funcs[];

void usage(void);
long get_next_time(char *snapshot_times);
void generate_stills(fdtd_2d_t *fdtd, char *snapshot_times, iterate_func_t iterate, render_func_t render);
void generate_video(fdtd_2d_t *fdtd, long duration, iterate_func_t iterate, render_func_t render);

int main(int argc, char *argv[])
{
    fdtd_2d_t fdtd;
    char *output_path = NULL;
    double gamma = -1.0f;
    int c = 0;
    int option_index = 0;
    enum {
        UNDEFINED,
        STILLS,
        VIDEO
    } function = UNDEFINED;
    int duration = 0;
    char *snapshot_times;
    long section = 5;
    iterate_func_t iterate = iterate_funcs[section];
    render_func_t render = render_funcs[section];
    
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, optstring, long_options, &option_index);
        if (EOF == c) break;
        switch (c) {
            case 'o':
                output_path = optarg;
                break;
            case 'g':
                gamma = atof(optarg);
                break;
            case 'v':
                if (function == UNDEFINED) {
                    function = VIDEO;
                    duration = atol(optarg);
                } else {
                    fprintf(stderr, "Only one of the options [-v, -s] can be selected at a time\n");
                }
                break;
            case 's':
                if (function == UNDEFINED) {
                    function = STILLS;
                    snapshot_times = optarg;
                } else {
                    fprintf(stderr, "Only one of the options [-v, -s] can be selected at a time\n");
                }
                break;
            case 'f':
                section = atol(optarg);
                if (section >= 3 && section <= 9) {
                    iterate = iterate_funcs[section];
                    render = render_funcs[section];
                }
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }
    if (gamma < 0) {
        /* This is the limit of 2D numerical stability */
        gamma = M_SQRT1_2;
    }
    
    FDTD_2D_init(&fdtd, gamma);

    switch (function) {
        case VIDEO:
            generate_video(&fdtd, duration, iterate, render);
            break;
        case STILLS:
            generate_stills(&fdtd, snapshot_times, iterate, render);
            break;
        case UNDEFINED:
        default:
            fprintf(stderr, "One of the options [-v, -s] must be selected\n");
            usage();
    }
    return 0;
}

long get_next_time(char *snapshot_times)
{
    static short first_time = 1;
    static char *c = NULL;
    long n = 0;
    if (1 == first_time) {
        c = snapshot_times;
        first_time = 0;
    }
    if ('\0' == *c) {
        return -1;
    } else {
        n = atol(c);
        while ((',' != *c) && ('\0' != *c)) ++c;
        if ('\0' != *c) ++c;
        return n;
    }
}

void generate_stills(fdtd_2d_t *fdtd, char *snapshot_times, iterate_func_t iterate, render_func_t render)
{
    cairo_surface_t *cs = NULL;
    cairo_t *c = NULL;
    char filename[MAX_PATH];
    long n = 0;
    long next_n = get_next_time(snapshot_times);
    while(next_n != -1) {
        n = iterate(fdtd);
        if (n == next_n * fdtd->N_P) {
            sprintf(filename, "/tmp/pics/%ld.svg", n);
            cs = cairo_svg_surface_create(filename, fdtd->I, fdtd->J);
            c = cairo_create(cs);
            render(c, fdtd);
            cairo_set_source_rgb(c, 0.0f, 0.0f, 0.0f);
            cairo_set_line_width(c, 0.25f);
            cairo_rectangle(c, 0.5f, 0.5f, fdtd->I, fdtd->J);
            cairo_stroke(c);
            cairo_destroy(c);
            cairo_surface_destroy(cs);
            next_n = get_next_time(snapshot_times);
        }
    }
}

void generate_video(fdtd_2d_t *fdtd, long duration, iterate_func_t iterate, render_func_t render)
{
    FILE *fp = NULL;
    cairo_surface_t *cs = NULL;
    cairo_t *c = NULL;
    video_state_t video_state;
    th_ycbcr_buffer ycbcr;
    long n = 0;
    
    fp = fopen("/tmp/movie.ogv", "w");
    if (NULL == fp) {
        fprintf(stderr, "ERROR<MAIN>: Could not file for writing the movie\n");
        exit(1);
    }

    cs = cairo_image_surface_create(CAIRO_FORMAT_RGB24, fdtd->I, fdtd->J+1);
    c = cairo_create(cs);

    if (VIDEO_init(&video_state, fdtd->I, fdtd->J+1) != VIDEO_OK) {
        fclose(fp);
        exit(1);
    }
    if (VIDEO_allocate_ycbcr_buffer(ycbcr, cs) != VIDEO_OK) {
        fclose(fp);
        exit(1);
    }
    
    /* Now the calculation is in steps */
    n = 0;
    while (n < duration) {
        n = iterate(fdtd);
        cairo_set_source_rgb(c, 1.0f, 1.0f, 1.0f);
        cairo_rectangle(c, 0, 0, fdtd->I, fdtd->J+1);
        cairo_fill(c);
        render(c, fdtd);
        if (VIDEO_rgb2ycbcr(cs, ycbcr) != VIDEO_OK) {
            goto free_all;
        }
        if (VIDEO_write_frame(&video_state, ycbcr, n == duration, fp) != VIDEO_OK) {
            goto free_all;
        }
    }

    /* Write whatever pages were left. XXX: Not sure if this is needed */
    if (VIDEO_write_pages(&video_state, fp)) {
        fclose(fp);
        return;
    }

free_all:
    cairo_destroy(c);
    cairo_surface_destroy(cs);
    fclose(fp);
    free(ycbcr[0].data);
    free(ycbcr[1].data);
    free(ycbcr[2].data);
}

void usage(void)
{
    fprintf(stderr, "USAGE: main [gamma] [[-v [duration] | -p [list of snapshot times]]\n");
}
