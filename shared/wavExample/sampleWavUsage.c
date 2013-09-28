/**************************************
 * sampleWavUsage.c
 * 
 * Very simple usage example of using 
 * the wav functions.
 **************************************/

#include "../wav.h"

int main(int argc, char **argv)
{
  WavFile *wav = NULL;

  if (argc != 2)
  {
    printf("Usage: ./sample song.wav\n\n");
    return EXIT_FAILURE;
  }

  wav = createWav(argv[1]);
  if (!loadWavData(wav))
  {
    deleteWav(wav);
    return EXIT_FAILURE;
  }

  playWav(wav);
  deleteWav(wav);

  return EXIT_SUCCESS;
}
