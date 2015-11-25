/*
 * Copyright © 2011 Mozilla Foundation
 *
 * This program is made available under an ISC-style license.  See the
 * accompanying file LICENSE for details.
 */

/* libcubeb api/function test. Plays a simple tone. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <cubeb/cubeb.h>
#include "common.h"

#define CAPTURE_SIZE_MS   500
#define CAPTURE_CHANNELS  1
//#define DUMPFILE "/tmp/cubeb_capture"
#ifdef DUMPFILE
static FILE * dump;
#endif

static cubeb_stream_params params;

void state_cb(cubeb_stream *stream, void *user, cubeb_state state)
{
  if (stream == NULL || user == NULL)
    return;

  switch (state) {
    case CUBEB_STATE_STARTED:
      printf("stream started\n");
      break;
    case CUBEB_STATE_STOPPED:
      printf("stream stopped\n");
      break;
    case CUBEB_STATE_DRAINED:
      printf("stream drained\n");
      break;
    default:
      printf("unknown stream state %d\n", state);
      break;
  }
}

long data_cb(cubeb_stream * stream, void * user, void * in, void * out, long n)
{
  if (stream == NULL || user == NULL)
    return CUBEB_ERROR;

  assert(in != NULL);
  assert(out == NULL);

  *(long *)(user) += n;

#ifdef DUMPFILE
  fwrite(in, params.channels*sizeof(int16_t), n, dump);
#endif

  return n;
}

int main(int argc, char *argv[])
{
  cubeb *ctx;
  cubeb_stream *stream;
  long total_frames = 0;
  uint32_t latency = 110, rate;
  int r;

#ifdef DUMPFILE
  dump = fopen("/tmp/haaspors-mic-dump", "w");
#endif

  r = cubeb_init(&ctx, "Cubeb tone example");
  if (r != CUBEB_OK) {
    fprintf(stderr, "Error initializing cubeb library\n");
    return r;
  }

  params.format = CUBEB_SAMPLE_S16NE;
  cubeb_get_preferred_sample_rate(ctx, &params.rate);
  params.channels = CAPTURE_CHANNELS;

  r = cubeb_stream_init(ctx, &stream, "Record (mono)", &params, NULL,
      latency, data_cb, state_cb, &total_frames);
  if (r != CUBEB_OK) {
    fprintf(stderr, "Error initializing cubeb stream\n");
    return r;
  }

  printf("latency: %u rate: %u ch: %u\n", latency, params.rate, params.channels);

  cubeb_stream_start(stream);
  delay(latency);
  delay(CAPTURE_SIZE_MS);
  cubeb_stream_stop(stream);

  cubeb_stream_destroy(stream);
  cubeb_destroy(ctx);

  printf("Number of frames captured: %lu (> %u ?)\n",
      total_frames, (params.rate * CAPTURE_SIZE_MS) / 1000);
  assert(total_frames >= (params.rate * CAPTURE_SIZE_MS) / 1000);

#ifdef DUMPFILE
  fclose(dump);
#endif

  return CUBEB_OK;
}
