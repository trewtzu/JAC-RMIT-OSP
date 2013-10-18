/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'conner S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 * * * * * * * * * * * * * * * * * * * * * */

#include "wav.h"

/* List of riff chunks that the program will recognise */
static const ChunkType knownChunks[] = {
  { "fmt ", readFormatChunk },
  { "data", readDataChunk }
};

WavFile* createWav(char *filename)
{
  WavFile *wav = NULL;

  /* Allocate memory for new WavFile struct */
  wav = safe_malloc(sizeof(WavFile));

  /* Copy the filename into the struct */
  wav->filename = safe_strdup(filename);

  /* Initialise struct members */
  wav->size = 0;
  wav->format = NULL;
  wav->data = NULL;

  return wav;
}

void deleteWav(WavFile *wav)
{
  if (wav == NULL)
    return;

  free(wav->filename);
  free(wav->format);

  if (wav->data != NULL) {
    free(wav->data->data);
    free(wav->data);
  }

  free(wav);
}

bool loadWavData(WavFile *wav)
{
  FILE *wavFile = NULL;
  char chunkId[CHUNK_ID_LEN];

  /* Open the wave file */
  wavFile = fopen(wav->filename, "rb");
  if (wavFile == NULL)
  {
    fprintf(stderr, "Error: Could not open '%s'\n", wav->filename);
    return false;
  }

  /* Check for RIFF ID */
  fread(chunkId, 1, CHUNK_ID_LEN, wavFile);
  if (!chunkIdCompare("RIFF", chunkId))
  {
    fprintf(stderr, "Error: File is not in RIFF format\n");
    fclose(wavFile);
    return false;
  }

  /* Read in file size */
  wav->size = bytesToUint(wavFile, NBYTES_SIZE);

  /* Check for WAVE ID */
  fread(chunkId, 1, CHUNK_ID_LEN, wavFile);
  if (!chunkIdCompare("WAVE", chunkId))
  {
    fprintf(stderr, "Error: File is not in WAVE format\n");
    fclose(wavFile);
    return false;
  }

  /* Read in all chunks in the file */
  if (!readChunks(wav, wavFile))
    return false;

  fclose(wavFile);
  
  /* A format chunk is required */
  if (wav->format == NULL)
  {
    fprintf(stderr, "Error: Format chunk not found\n");
    return false;
  }

  /* A data chunk is also required */
  if (wav->data == NULL)
  {
    fprintf(stderr, "Error: Data chunk not found\n");
    return false;
  }

  return true;
}

bool readChunks(WavFile *wav, FILE *file)
{
  char chunkId[CHUNK_ID_LEN];
  int numKnownChunks;

  /* Calculate the how many known chunk types we have.
   * Could have a global/constant for this but this way it's
   * easier to add support for more chunk types */
  numKnownChunks = sizeof(knownChunks) / sizeof(ChunkType);

  /* Keep reading while there is still a chunk id available */
  while (fread(chunkId, 1, CHUNK_ID_LEN, file) == CHUNK_ID_LEN)
  {
    int i;
    bool matched = false;
    ChunkType chunk;

    /* Check if the ID is supported */
    for (i = 0; i < numKnownChunks; ++i)
    {
      chunk = knownChunks[i];
      
      if (chunkIdCompare(chunkId, chunk.id))
      {
        /* Read in the chunks data */
        if (!chunk.chunkFunc(wav, file))
          return false;

        matched = true;
        break;
      }
    }
    
    /* Chunk is not supported, so just skip it */
    if (!matched)
    {
      uint skip = bytesToUint(file, NBYTES_SIZE);
      fseek(file, skip, SEEK_CUR);
    }
  }

  return true;
}

bool readFormatChunk(WavFile *wav, FILE *file)
{
  FormatChunk *format = NULL;
  uint numExtraBytes = 0;

  /* Allocate memory */
  format = safe_malloc(sizeof(FormatChunk));

  /* Get fmt size */
  format->size = bytesToUint(file, NBYTES_SIZE);

  /* Get each format attribute */
  format->compressionCode = bytesToUint(file, NBYTES_COMP_CODE);
  format->numChannels = bytesToUint(file, NBYTES_CHANNELS);
  format->sampleRate = bytesToUint(file, NBYTES_SAMPLE_RATE);
  format->averageBPS = bytesToUint(file, NBYTES_AVERAGE_BPS);
  format->blockAlign = bytesToUint(file, NBYTES_BLOCK_ALIGN);
  format->significantBPS = bytesToUint(file, NBYTES_SIGNIFICANT_BPS);

  /* Skip over extra format bytes if they exist*/
  if (format->size > NBYTES_FORMAT_BASE)
  {
    numExtraBytes = bytesToUint(file, NBYTES_EXTRA_FORMAT);
    fseek(file, numExtraBytes, SEEK_CUR);
  }

  wav->format = format;

  return true;
}

bool readDataChunk(WavFile *wav, FILE *file)
{
  DataChunk *data = NULL;

  /* Allocate memory for new DataChunk struct*/
  data = safe_malloc(sizeof(DataChunk));

  /* Get data size */
  data->size = bytesToUint(file, NBYTES_SIZE);

  /* Allocate enough memory to store all data */
  data->data = safe_malloc(data->size * sizeof(unsigned char));

  /* Read data */
  fread(data->data, 1, data->size, file);

  wav->data = data;

  return true;
}

bool playWav(WavFile *wav)
{
  snd_pcm_t *handle;
  snd_pcm_uframes_t periodFrames, framesWritten, framesLeft;
  uint frameSize;
  unsigned char *buffer;
  long n;

  handle = initWavPlayback(wav->format, &periodFrames);
  if (handle == NULL)
    return false;

  frameSize = getFrameSize(wav->format);

  /* Write all frames to the sound device */
  framesWritten = 0;
  framesLeft = wav->data->size / frameSize;
  while (framesLeft > 0)
  {
    snd_pcm_uframes_t availFrames = 8 * periodFrames;
    if (framesLeft < availFrames)
    {
      availFrames = framesLeft;
    }

    buffer = wav->data->data + (framesWritten * frameSize);

    n = bufferWav(handle, buffer, availFrames);
    if (n == -1)
    {
      break;
    }
    
    framesWritten += n;
    framesLeft -= n;
  }

  stopWavPlayback(handle);

  return true;
}

snd_pcm_t *initWavPlayback(FormatChunk *format, snd_pcm_uframes_t *period)
{
  int err, dir;
  uint exactRate;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;

  /* Open PCM device for playback */
  err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0)
  {
    fprintf(stderr, "Error: Unable to open pcm device; %s\n",
      snd_strerror(err));
    return false;
  }

  /* Allocate hardware parameters object */
  snd_pcm_hw_params_malloc(&params);

  /* Load default hardware parameters */
  snd_pcm_hw_params_any(handle, params);

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little endian */
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

  /* Set number of audio channels to use */
  snd_pcm_hw_params_set_channels(handle, params, format->numChannels);

  /* Set the sample rate as close as possible */
  exactRate = format->sampleRate;
  snd_pcm_hw_params_set_rate_near(handle, params, &exactRate, &dir);
  if (exactRate != format->sampleRate)
  {
      fprintf(stderr, "Unable to set rate to %u. Using %u instead.\n",
          format->sampleRate, exactRate);
  }

  /* Write parameters to the driver */
  err = snd_pcm_hw_params(handle, params);
  if (err < 0)
  {
    fprintf(stderr, "Unable to set hardware parameters: %s",
      snd_strerror(err));
    snd_pcm_close(handle);
    handle = NULL;
  }

  /* Get the number of frames in one period */
  if (period != NULL)
  {
    snd_pcm_hw_params_get_period_size(params, period, &dir);
  }

  /* No longer need hw params */
  snd_pcm_hw_params_free(params);

  return handle;
}

void stopWavPlayback(snd_pcm_t *handle)
{
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
}

long bufferWav(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t numFrames)
{
  long framesWritten;

  framesWritten = snd_pcm_writei(handle, buffer, numFrames);
  if (framesWritten == -EPIPE)
  {
    fprintf(stderr, "Buffer underrun occurred\n");
    snd_pcm_prepare(handle);
  }
  else if (framesWritten < 0)
  {
    fprintf(stderr, "Failed to write to pcm device: %s\n",
        snd_strerror(framesWritten));
    return -1;
  }
  else if (framesWritten != numFrames)
  {
    fprintf(stderr, "Short write occurred\n");
  }

  return framesWritten;
}

bool chunkIdCompare(char *id1, char *id2)
{
  int i;

  for (i = 0; i < CHUNK_ID_LEN; ++i)
  {
    if (id1[i] != id2[i])
      return false;
  }

  return true;
}

uint bytesToUint(FILE *file, uint n)
{
  unsigned char *bytes;
  uint i, value = 0;

  if (n == 0)
    return 0;

  bytes = safe_malloc(n * sizeof(unsigned char));

  if (fread(bytes, 1, n, file) != n) {
    fprintf(stderr, "Error: Could not read enough bytes\n");
    return 0;
  }

  for (i = 0; i < n; ++i)
    value |= bytes[i] << (8 * i);

  free(bytes);

  return value;
}

uint getFrameSize(FormatChunk *format)
{
  int bytesPerSample = format->significantBPS / 8;
  uint frameSize = bytesPerSample * format->numChannels;

  return frameSize;
}
