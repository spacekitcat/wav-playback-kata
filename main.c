#include <stdio.h>
#include <stdlib.h>
#include <AudioUnit/AudioUnit.h>
#include "portaudio.h"

struct RIFF
{
  char chunkID[4];
  int chunkSize;
  char format[4];
};

struct WAV
{
  char formatChunkId[4];
  int formatChunkSize;
  short audioFormat;
  short numChannels;
  int sampleRate;
  int byteRate;
  short blockAlign;
  short bitsPerSample;
  char dataChunkID[4];
  int dataChunkSize;
};

typedef struct
{
  float left_phase;
  float right_phase;
} paTestData;
/* This routine will be called by the PortAudio engine when audio is needed.
   It may called at interrupt level on some machines so don't do anything
   that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
  /* Cast data passed through stream to our structure. */
  paTestData *data = (paTestData *)userData;
  float *out = (float *)outputBuffer;
  unsigned int i;
  (void)inputBuffer; /* Prevent unused variable warning. */

  for (i = 0; i < framesPerBuffer; i++)
  {
    *out++ = data->left_phase;  /* left */
    *out++ = data->right_phase; /* right */
    /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
    data->left_phase += 0.01f;
    /* When signal reaches top, drop back down. */
    if (data->left_phase >= 1.0f)
      data->left_phase -= 2.0f;
    /* higher pitch so we can distinguish left and right. */
    data->right_phase += 0.03f;
    if (data->right_phase >= 1.0f)
      data->right_phase -= 2.0f;
  }
  return 0;
}

struct RIFF *readRiffHeader(FILE *handle)
{
  struct RIFF *writeTarget;

  writeTarget = malloc(sizeof(struct RIFF));

  fread(writeTarget, sizeof(struct RIFF), 1, handle);

  return writeTarget;
}

struct WAV *readWavHeader(FILE *handle)
{
  struct WAV *writeTarget;

  writeTarget = malloc(sizeof(struct WAV));

  fread(writeTarget, sizeof(struct WAV), 1, handle);

  return writeTarget;
}

int main(int argc, char **argv)
{
  char inputFile[] = "sample.wav\0";
  FILE *handle = fopen(inputFile, "r");

  if (!handle)
  {
    printf("Failed to open WAV file");
    return 1;
  }

  printf("Opened %s for reading\n", inputFile);

  struct RIFF *riffHeader = readRiffHeader(handle);

  printf("ChunkID\t\t= %.*s\n", 4, riffHeader->chunkID);
  printf("ChunkSize\t= %d\n", riffHeader->chunkSize);
  printf("Format\t\t= %.*s\n", 4, riffHeader->format);

  struct WAV *wavHeader = readWavHeader(handle);
  printf("FormatChunkId\t= %.*s\n", 4, wavHeader->formatChunkId);
  printf("FormatChunkSize\t= %d\n", wavHeader->formatChunkSize);
  printf("audioFormat\t= %d\n", wavHeader->audioFormat);
  printf("numChannels\t= %d\n", wavHeader->numChannels);
  printf("sampleRate\t= %d\n", wavHeader->sampleRate);
  printf("byteRate\t= %d\n", wavHeader->byteRate);
  printf("blockAlign\t= %d\n", wavHeader->blockAlign);
  printf("bitsPerSample\t= %d\n", wavHeader->bitsPerSample);
  printf("DataChunkId\t= %.*s\n", 4, wavHeader->dataChunkID);
  printf("DataChunkSize\t= %d\n", wavHeader->dataChunkSize);

  char *dataBuffer = malloc(wavHeader->dataChunkSize);
  char *dataBufferPtr = dataBuffer;

  printf("%d\n", wavHeader->dataChunkSize);
  printf("%d\n", wavHeader->byteRate);
  int byteReadRate = wavHeader->sampleRate / wavHeader->bitsPerSample;
  for (int i = 0; i < wavHeader->dataChunkSize; i += byteReadRate)
  {
    char next[wavHeader->byteRate];
    fread(dataBufferPtr, byteReadRate, 1, handle);
    //memcpy(dataBufferPtr, next, byteReadRate);
    dataBufferPtr += byteReadRate;
  }

  if (!Pa_Initialize())
  {
    printf("Failed to initialise portaudio");
    return 1;
  }

  free(riffHeader);
  fclose(handle);

  return 0;
}
