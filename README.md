# untitled WAV experiments

Exploring WAV and PCM seems as good as any other rabbit hole for my fix of almost useless knowledge about computers.

## Journal

### 16 July

WAV files are pretty interesting, but I'm not sure how I got here. I was reading about the relationships between the same notes at different octaves. The frequency of each note just about doubles between every octave, where you have 32Hz at C1, you have 65Hz at C2, 130Hz at C3, 261Hz at C4, and so on. Then I was reading about the concept of ratios in describing certain note qualities (major, minor, diminished, augmented), and I suppose from there I started wondering what (if anything) I could learn about music theory by figuring out how to generate and write raw sound data to the sound card.

WAV uses Resource Interchange File Format (RIFF) as a container. The first 'chunk' within the RIFF file is the WAV header and the second 'chunk' is the raw Linear Pulse Code Modulation (LPCM) data. My understanding is that the amplitude of an analog audio signal is sampled at a fixed frequency and each reading quantized to the nearest value on some fixed scale. which gives you a Pulse Code Modulation (PCM) stream. The data chunk in a WAV file is just an LPCM stream.

I started out trying to use the AudioUnit API in OSX, but it didn't seem worth it. Apple don't seem to have much interest in supporting those SDKs for C the way they do for their own languages. PortAudio seems like a fair compromise and should at least make the code portable.

I've written a parser in C, it reads the RIFF and WAV headers and copies the LPCM stream into a buffer. I've not tested the last part yet, I'm working on getting PortAudio setup first.

### 17 July

I got it to play the PCM data without distortion and an appropriate base signal level (volume). I remember there was this idea that variable definitions should be at the top of the method in C. I'm not sure if that really makes sense, surely you'd want variables defined near their use (and also minimize the distance between memory allocation and memory deallocation calls to make it easier to avoid mistakes). I've considered that it might actually be some sort of optimisation strategy for the compiler? I assumed modern compilers were smart enough to be able to not give a flying duck.

### 18 July

I cleaned the code up a little bit, added Wave format (the wave data encoding) detection. It can only play Microsoft PCM files and the signal correction
code isn't technically correct, but it does work! I might explore a more technically correct implementation for MS PCM and some kind of working
implementation for IMA ADPCM, the latter is a mini project in its own right while the former is probably an afternoon project.  

## Building

You need to install PortAudio as a shared library, on OSX you can use Brew.

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

## Sources

[Multimedia Programming Interface
and Data Specifications 1.0 (IBM and Microsoft, 1991)](http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/Docs/riffmci.pdf)

[IMA ADPCM wiki page](https://wiki.multimedia.cx/index.php/IMA_ADPCM)

[IMA ADPCM specification](http://www.cs.columbia.edu/~hgs/audio/dvi/IMA_ADPCM.pdf)

[PortAudio Documentation](http://portaudio.com/docs/v19-doxydocs/)
