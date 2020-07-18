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
  RIFFCkHeader riffChunkHeader;
  char format[4];

} RIFFHeader;

typedef struct WavHeader
{
  RIFFCkHeader waveFormatChunkHeader;
  short waveFormatTag;
  short waveChannels;
  int waveSamplesPerSecond;
  int waveAvgBytesPerSecond;
  short waveBlockAlign;
  short bitsPerSample;
  RIFFCkHeader waveDataChunkHeader;

} WavHeader;

typedef struct PcmDataChunk
{
  short int *buffer;
  short int *iterator;
  struct WavHeader *wavHeader;

} PcmDataChunk;

enum WaveFormatTypes
{
  MICROSOFT_PCM = 0x0001,
  IBM_MULAW = 0x0101,
  IBM_ALAW = 0x0102,
  IBM_ADPCM = 0x0103,
  ADPCM = 0x0003
};

float correctSignalPowerLevel(float signal, WavHeader *format, int fixedReductionBias)
{
  return (signal / format->waveSamplesPerSecond / format->waveChannels / format->waveBlockAlign) - fixedReductionBias;
}

static int streamProcessorCb(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *input)
{
  float *out = (float *)outputBuffer;
  PcmDataChunk *PcmDataChunk = (struct PcmDataChunk *)input;

  for (unsigned int i = 0; i < framesPerBuffer * PcmDataChunk->wavHeader->waveChannels; i++)
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
  printf("riffHeader->riffChunkHeader.ckID\t= %.*s\n", 4, riffHeader->riffChunkHeader.ckID);
  printf("riffHeader->riffChunkHeader.ckSize\t= %d\n", riffHeader->riffChunkHeader.ckSize);
  printf("riffHeader->format\t\t= %.*s\n", 4, riffHeader->format);

  printf("wavHeader->waveFormatChunkHeader.ckID\t= %.*s\n", 4, wavHeader->waveFormatChunkHeader.ckID);
  printf("avHeader->waveFormatChunkHeader.ckSize\t= %d\n", wavHeader->waveFormatChunkHeader.ckSize);
  printf("wavHeader->waveFormatTag\t\t= %d\n", wavHeader->waveFormatTag);
  printf("wavHeader->waveChannels\t\t= %d\n", wavHeader->waveChannels);
  printf("wavHeader->waveSamplesPerSecond\t\t= %d\n", wavHeader->waveSamplesPerSecond);
  printf("wavHeader->waveAvgBytesPerSecond\t\t= %d\n", wavHeader->waveAvgBytesPerSecond);
  printf("wavHeader->waveBlockAlign\t\t= %d\n", wavHeader->waveBlockAlign);
  printf("wavHeader->bitsPerSample\t= %d\n", wavHeader->bitsPerSample);
  printf("wavHeader->waveDataChunkHeader.ckID\t= %.*s\n", 4, wavHeader->waveDataChunkHeader.ckID);
  printf("wavHeader->waveDataChunkHeader.ckSize\t= %d\n", wavHeader->waveDataChunkHeader.ckSize);
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

  printf("\n\nWARNING: Only Microsoft PCM support is implemented. ");
  switch (wavHeader->waveFormatTag)
  {
  case IBM_MULAW:
    printf("This is IBM mu-law format.\n\n");
    break;
  case IBM_ALAW:
    printf("This is IBM a-law format.\n\n");
    break;
  case IBM_ADPCM:
    printf("This is IBM AVC Adaptive Differential Pulse Code Modulation format.\n\n");
    break;
  case ADPCM:
    printf("This is IMA ADPCM format.\n\n");
    break;
  case MICROSOFT_PCM:
    printf("This is Microsoft PCM format.\n\n");
    break;
  default:
    printf("This format is unrecognised.\n\n");
    break;
  }

  dumpHeaders(riffHeader, wavHeader);

  int paInitResult = Pa_Initialize();
  if (paInitResult != 0)
  {
    printf("Failed to initialise portaudio, %s\n", Pa_GetErrorText(paInitResult));
    return 1;
  }

  short int *dataBuffer = malloc(wavHeader->waveDataChunkHeader.ckSize);
  short int *dataBufferPtr = dataBuffer;

  fread(dataBufferPtr, wavHeader->waveDataChunkHeader.ckSize, 1, handle);

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
      wavHeader->waveSamplesPerSecond,
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

  int duration = wavHeader->waveDataChunkHeader.ckSize / wavHeader->waveAvgBytesPerSecond;
  printf("\nFile duration: %d minutes and %d seconds\n\n", duration / 60, duration % 60);
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
