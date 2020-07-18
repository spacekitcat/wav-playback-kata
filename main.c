#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <portaudio.h>

typedef struct RIFFCkHeader
{
  char ckID[4];
  int ckSize;

} RIFFCkHeader;

typedef struct RIFFHeader
{
  RIFFCkHeader ckHeader;
  char format[4];

} RIFFHeader;

typedef struct WavHeader
{
  RIFFCkHeader ckFormatHeader;
  short audioFormat;
  short numChannels;
  int sampleRate;
  int byteRate;
  short blockAlign;
  short bitsPerSample;
  RIFFCkHeader ckDataHeader;

} WavHeader;

typedef struct PcmDataChunk
{
  short int *buffer;
  short int *iterator;
  struct WavHeader *wavHeader;

} PcmDataChunk;

float correctSignalPowerLevel(float signal, WavHeader *format, int fixedReductionBias)
{
  return (signal / format->sampleRate / format->numChannels / format->blockAlign) - fixedReductionBias;
}

static int streamProcessorCb(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *input)
{
  float *out = (float *)outputBuffer;
  PcmDataChunk *PcmDataChunk = (struct PcmDataChunk *)input;

  for (unsigned int i = 0; i < framesPerBuffer * PcmDataChunk->wavHeader->numChannels; i++)
  {
    *out++ = correctSignalPowerLevel(*PcmDataChunk->iterator++, PcmDataChunk->wavHeader, 0);
  }

  return 0;
}

RIFFHeader *readRiffHeader(FILE *handle)
{
  RIFFHeader *writeTarget = malloc(sizeof(RIFFHeader));
  fread(writeTarget, sizeof(RIFFHeader), 1, handle);
  return writeTarget;
}

WavHeader *readWavHeader(FILE *handle)
{
  WavHeader *writeTarget = malloc(sizeof(WavHeader));
  fread(writeTarget, sizeof(WavHeader), 1, handle);
  return writeTarget;
}

void dumpHeaders(RIFFHeader *riffHeader, WavHeader *wavHeader)
{
  printf("riffHeader->ckHeader.ckID\t= %.*s\n", 4, riffHeader->ckHeader.ckID);
  printf("riffHeader->ckHeader.ckSize\t= %d\n", riffHeader->ckHeader.ckSize);
  printf("riffHeader->format\t\t= %.*s\n", 4, riffHeader->format);

  printf("wavHeader->ckFormatHeader.ckID\t= %.*s\n", 4, wavHeader->ckFormatHeader.ckID);
  printf("avHeader->ckFormatHeader.ckSize\t= %d\n", wavHeader->ckFormatHeader.ckSize);
  printf("wavHeader->audioFormat\t\t= %d\n", wavHeader->audioFormat);
  printf("wavHeader->numChannels\t\t= %d\n", wavHeader->numChannels);
  printf("wavHeader->sampleRate\t\t= %d\n", wavHeader->sampleRate);
  printf("wavHeader->byteRate\t\t= %d\n", wavHeader->byteRate);
  printf("wavHeader->blockAlign\t\t= %d\n", wavHeader->blockAlign);
  printf("wavHeader->bitsPerSample\t= %d\n", wavHeader->bitsPerSample);
  printf("wavHeader->ckDataHeader.ckID\t= %.*s\n", 4, wavHeader->ckDataHeader.ckID);
  printf("wavHeader->ckDataHeader.ckSize\t= %d\n", wavHeader->ckDataHeader.ckSize);
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("\nIncorrect number of parameters.\n");
    printf("\tUSAGE:\n\t\t./wavout <path-to-wav-file>\n\n");
    return 1;
  }

  char *inputFile = argv[1];
  FILE *handle = fopen(inputFile, "rb");

  if (!handle)
  {
    printf("Failed to open input file %s\n", inputFile);
    return 1;
  }

  RIFFHeader *riffHeader = readRiffHeader(handle);
  WavHeader *wavHeader = readWavHeader(handle);

  if (wavHeader->audioFormat != 1)
  {
    printf("\n\nWARNING: I only know how to play LPCM, this appears to have some sort of compression. Format code: %d\n\n", wavHeader->audioFormat);
  }

  dumpHeaders(riffHeader, wavHeader);

  int paInitResult = Pa_Initialize();
  if (paInitResult != 0)
  {
    printf("Failed to initialise portaudio, %s\n", Pa_GetErrorText(paInitResult));
    return 1;
  }

  short int *dataBuffer = malloc(wavHeader->ckDataHeader.ckSize);
  short int *dataBufferPtr = dataBuffer;

  fread(dataBufferPtr, wavHeader->ckDataHeader.ckSize, 1, handle);

  PaStreamParameters outputParams;
  outputParams.device = Pa_GetDefaultOutputDevice();
  outputParams.channelCount = 2;
  outputParams.hostApiSpecificStreamInfo = NULL;
  outputParams.sampleFormat = paFloat32;
  outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;

  PcmDataChunk data;
  data.buffer = dataBuffer;
  data.iterator = dataBuffer;
  data.wavHeader = wavHeader;

  PaStream *stream;
  int err = Pa_OpenStream(
      &stream,
      NULL,
      &outputParams,
      wavHeader->sampleRate,
      2048,
      paClipOff,
      streamProcessorCb,
      &data);

  if (err != 0)
  {
    printf("Failed to to open byte stream, %s\n", Pa_GetErrorText(err));
    return 1;
  }

  err = Pa_StartStream(stream);
  if (err != 0)
  {
    printf("Failed to start stream, %s\n", Pa_GetErrorText(err));
    return 1;
  }

  int duration = wavHeader->ckDataHeader.ckSize / wavHeader->byteRate;
  printf("\nFile duration: %d minutes and %d seconds\n\n", duration/60, duration%60);
  Pa_Sleep(duration * 1000);

  err = Pa_CloseStream(stream);
  if (err != 0)
  {
    printf("Failed to close stream, %s\n", Pa_GetErrorText(err));
    return 1;
  }

  err = Pa_Terminate();
  if (err != 0)
  {
    printf("Failed to terminate portaudio, %s\n", Pa_GetErrorText(err));
    return 1;
  }

  free(dataBuffer);
  free(wavHeader);
  free(riffHeader);
  fclose(handle);

  return 0;
}
