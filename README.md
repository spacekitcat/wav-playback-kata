# untitled WAV experiments

Exploring WAV and PCM seems as good as any other rabbit hole for my fix of almost useless knowledge about computers.

## Journal

### 16 July

WAV files are pretty interesting, but I'm not sure how I got here. I was reading about the relationships between the same notes at different octaves. The frequency of each note just about doubles between every octave, where you have 32Hz at C1, you have 65Hz at C2, 130Hz at C3, 261Hz at C4, and so on. Then I was reading about the concept of ratios in describing certain note qualities (major, minor, diminished, augmented), and I suppose from there I started wondering what (if anything) I could learn about music theory by figuring out how to generate and write raw sound data to the sound card.

WAV uses Resource Interchange File Format (RIFF) as a container. The first 'chunk' within the RIFF file is the WAV header and the second 'chunk' is the raw Linear Pulse Code Modulation (LPCM) data. My understanding is that the amplitude of an analog audio signal is sampled at a fixed frequency and each reading quantized to the nearest value on some fixed scale. which gives you a Pulse Code Modulation (PCM) stream. The data chunk in a WAV file is just an LPCM stream.

I started out trying to use the AudioUnit API in OSX, but it didn't seem worth it. Apple don't seem to have much interest in supporting those SDKs for C the way they do for their own languages. Portaudio seems like a fair compromise and should at least make the code portable.

I've written a parser in C, it reads the RIFF and WAV headers and copies the LPCM stream into a buffer. I've not tested the last part yet, I'm working on getting portaudio setup first.

## Building

You need to install Portaudio as a shared library, on OSX you can use Brew.

```bash
brew install portaudio
```

Optimistically, you should only have to check out the code and use the build scripts generated with Autotools.

```bash
git clone github.com/spacekitcat/<repository-name>
cd <repository-name>
./configure
make
```
