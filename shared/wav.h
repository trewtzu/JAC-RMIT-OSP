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
typedef struct format_chunk FormatChunk;

struct format_chunk {
  uint size;
  uint compressionCode;
  uint numChannels;
  uint sampleRate;
  uint averageBPS;
  uint blockAlign;
  uint significantBPS;
};

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

#endif
