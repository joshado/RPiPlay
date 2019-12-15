/**
 * RPiPlay - An open-source AirPlay mirroring server for Raspberry Pi
 * Copyright (C) 2019 Florian Draschbacher
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "video_renderer.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <signal.h>

struct video_renderer_s {
    logger_t *logger;
    int pid;
    int pipe;
};

video_renderer_t *video_renderer_init(logger_t *logger, bool background, bool low_latency) {
    video_renderer_t *renderer;
    renderer = calloc(1, sizeof(video_renderer_t));
    if (!renderer) {
        return NULL;
    }
    renderer->logger = logger;
    renderer->pid = 0;
    return renderer;
}

void video_renderer_start(video_renderer_t *renderer) {
    char *script = getenv("RendererScript");
    printf("renderer start\n");

    if(script) {

        int renderer_pipe[2];
        if( -1 == pipe(renderer_pipe) ) {
            // failed!
        }

        renderer->pid = fork();
        if(renderer->pid == -1) {
            // failed to fork!
        } else if(renderer->pid == 0) {
            close(renderer_pipe[1]);
            char pipe_arg[64];
            snprintf(pipe_arg, 64, "%i", renderer_pipe[0]);
            printf("Executing script %s %s\n", script, pipe_arg);
            close(STDIN_FILENO);
            dup2(renderer_pipe[0], STDIN_FILENO);

            (void) execlp(script, pipe_arg, (char *)0);
            perror("execlp failed");
            close(renderer_pipe[0]);
            exit(128);
        }

        close(renderer_pipe[0]);
        renderer->pipe = renderer_pipe[1];
    }
}

void video_renderer_render_buffer(video_renderer_t *renderer, raop_ntp_t *ntp, unsigned char* data, int data_len, uint64_t pts, int type) {
    printf("renderer buffer %llu\n", pts);
    if(renderer->pipe) {
        write(renderer->pipe, data, data_len);
    }
}

void video_renderer_flush(video_renderer_t *renderer) {
    printf("renderer flush\n");

}

void video_renderer_destroy(video_renderer_t *renderer) {

    printf("renderer destroy\n");
    if(renderer->pipe) {
        close(renderer->pipe);
        renderer->pipe = 0;
    }
    if(renderer->pid) {
        printf("Sending SIGHUP\n");
        kill(renderer->pid, SIGHUP);
        renderer->pid = 0;
    }

    if (renderer) {
        free(renderer);
    }


}
