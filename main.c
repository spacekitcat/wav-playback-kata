#include <stdio.h>
#include <stdlib.h>
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

struct paTestData
{
  float left_phase;
  float right_phase;
};

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
  float *out = (float *)outputBuffer;
  unsigned int i;
  (void)inputBuffer; /* Prevent unused variable warning. */

  printf("%d\n", inputBuffer);
  for (i = 0; i < framesPerBuffer; i++)
  {

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
  char dumpFile[] = "dump.bin\0";
  FILE *handle = fopen(inputFile, "rb");

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
  printf("Byte read rate: \t%d\n", byteReadRate);
  for (int i = 0; i < wavHeader->dataChunkSize; i += byteReadRate)
  {
    char next[wavHeader->byteRate];
    fread(dataBufferPtr, byteReadRate, 1, handle);
    dataBufferPtr += byteReadRate;
  }

  FILE *dumpFileHandle = fopen(dumpFile, "w");
  if (!dumpFileHandle)
  {
    printf("Failed to open dump file");
    return 1;
  }
  fwrite(dataBuffer, wavHeader->dataChunkSize, 1, dumpFileHandle);
  fclose(dumpFileHandle);

  int paInitResult = Pa_Initialize();
  if (paInitResult != 0)
  {
    printf("Failed to initialise portaudio, %s\n", Pa_GetErrorText(paInitResult));
    return 1;
  }
  else
  {
    printf("Initialised portaudio\n");
  }

  struct PsStream *stream;
  struct paTestData testData;

  int err = Pa_OpenDefaultStream(
      &stream,
      0,
      2,
      paFloat32,
      wavHeader->sampleRate,
      paFramesPerBufferUnspecified,
      patestCallback,
      &testData);

  if (err != 0)
  {
    printf("Failed to to open byte stream, %s\n", Pa_GetErrorText(err));
    return 1;
  }
  else
  {
    printf("Opened byte stream\n");
  }

  err = Pa_StartStream(stream);
  if (err != 0)
  {
    printf("Failed to start stream, %s\n", Pa_GetErrorText(err));
    return 1;
  }
  else
  {
    printf("Started stream\n");
  }

  Pa_WriteStream(stream, dataBuffer, 1000);

  Pa_Sleep(3 * 1000);

  err = Pa_CloseStream(stream);
  if (err != 0)
  {
    printf("Failed to close stream, %s\n", Pa_GetErrorText(err));
    return 1;
  }
  else
  {
    printf("Stopped  stream\n");
  }

  err = Pa_Terminate();
  if (err != 0)
  {
    printf("Failed to terminate portaudio, %s\n", Pa_GetErrorText(err));
    return 1;
  }
  else
  {
    printf("Stopped portaudio\n");
  }

  free(riffHeader);
  fclose(handle);

  return 0;
}
