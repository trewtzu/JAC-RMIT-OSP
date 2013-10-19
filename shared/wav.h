/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'Connor S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef WAV_H
#define WAV_H

#include <alsa/asoundlib.h>

#include "util.h"

/* Length of a chunk ID */
#define CHUNK_ID_LEN  4

/* Number of bytes used to store each attribute of the wave file */
#define NBYTES_SIZE             4   /* Chunk size */
#define NBYTES_COMP_CODE        2   /* Wave compression code */
#define NBYTES_CHANNELS         2   /* Number of channels */
#define NBYTES_SAMPLE_RATE      4   /* Sample rate */
#define NBYTES_AVERAGE_BPS      4   /* Average bytes per second */
#define NBYTES_BLOCK_ALIGN      2   /* Block align */
#define NBYTES_SIGNIFICANT_BPS  2   /* Significant bits per sample */
#define NBYTES_FORMAT_BASE      16  /* Total fmt chunk bytes excluding 
                                     * 'extra format bytes' */
#define NBYTES_EXTRA_FORMAT     2   /* Size of 'extra format bytes' */

typedef unsigned int uint;

typedef struct {
  uint size;
  uint compressionCode;
  uint numChannels;
  uint sampleRate;
  uint averageBPS;
  uint blockAlign;
  uint significantBPS;
} FormatChunk;

typedef struct {
  uint size;
  unsigned char *data;
} DataChunk;

typedef struct {
  char *filename;
  uint size;
  FormatChunk *format;
  DataChunk *data;
} WavFile;

typedef struct {
  char *id;
  bool (*chunkFunc)(WavFile *wav, FILE *file);
} ChunkType;

/* Helper functions */
bool chunkIdCompare(char *id1, char *id2);
uint bytesToUint(FILE *file, uint n);
uint getFrameSize(FormatChunk *format);

/* Create / delete */
WavFile* createWav(char *filename);
void deleteWav(WavFile *wav);

/* Data loading */
bool loadWavData(WavFile *wav);
bool readChunks(WavFile *wav, FILE *file);
bool readFormatChunk(WavFile *wav, FILE *file);
bool readDataChunk(WavFile *wav, FILE *file);

/* Playback */
bool playWav(WavFile *wav);
snd_pcm_t *initWavPlayback(FormatChunk *format, snd_pcm_uframes_t *period);
void stopWavPlayback(snd_pcm_t *handle);
long bufferWav(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t numFrames);

#endif
