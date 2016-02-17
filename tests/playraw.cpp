/******************************************/
/*
  playraw.cpp
  by Gary P. Scavone, 2007

  Play a specified raw file.  It is necessary
  that the file be of the same data format as
  defined below.
*/
/******************************************/

#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

/*
typedef char  MY_TYPE;
#define FORMAT RTAUDIO_SINT8
#define SCALE  127.0
*/

typedef signed short  MY_TYPE;
#define FORMAT RTAUDIO_SINT16
#define SCALE  32767.0

/*
typedef S24 MY_TYPE;
#define FORMAT RTAUDIO_SINT24
#define SCALE  8388607.0

typedef signed int  MY_TYPE;
#define FORMAT RTAUDIO_SINT32
#define SCALE  2147483647.0

typedef float  MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32
#define SCALE  1.0;

typedef double  MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64
#define SCALE  1.0;
*/

// Platform-dependent sleep routines.
#if defined( __WINDOWS_ASIO__ ) || defined( __WINDOWS_DS__ ) || defined( __WINDOWS_WASAPI__ )
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

void usage( void ) {
  // Error function in case of incorrect command-line
  // argument specifications
  std::cout << "\nuseage: playraw N fs file <device> <channelOffset>\n";
  std::cout << "    where N = number of channels,\n";
  std::cout << "    fs = the sample rate, \n";
  std::cout << "    file = the raw file to play,\n";
  std::cout << "    device = optional device to use (default = 0),\n";
  std::cout << "    and channelOffset = an optional channel offset on the device (default = 0).\n\n";
  exit( 0 );
}

struct OutputData {
  FILE *fd;
  unsigned int channels;
};

// Interleaved buffers
int output( void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
            double /*streamTime*/, RtAudioStreamStatus /*status*/, void *data )
{
  OutputData *oData = (OutputData*) data;

  // In general, it's not a good idea to do file input in the audio
  // callback function but I'm doing it here because I don't know the
  // length of the file we are reading.
  unsigned int count = fread( outputBuffer, oData->channels * sizeof( MY_TYPE ), nBufferFrames, oData->fd);
  if ( count < nBufferFrames ) {
    unsigned int bytes = (nBufferFrames - count) * oData->channels * sizeof( MY_TYPE );
    unsigned int startByte = count * oData->channels * sizeof( MY_TYPE );
    memset( (char *)(outputBuffer)+startByte, 0, bytes );
    return 1;
  }

  return 0;
}

extern short int samples[100000];
extern void testtone(INT16*out);


int DTMF(void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
	double /*streamTime*/, RtAudioStreamStatus status, void *data)
{


	//OutputData *oData = (OutputData*)data;

	// In general, it's not a good idea to do file input in the audio
	// callback function but I'm doing it here because I don't know the
	// length of the file we are reading.

	//unsigned int count = fread(outputBuffer, oData->channels * sizeof(MY_TYPE), nBufferFrames, oData->fd);
	unsigned int count = nBufferFrames;


	memcpy(outputBuffer, data, 100000);
	

	//if (count < nBufferFrames) {
	//	unsigned int bytes = (nBufferFrames - count) * oData->channels * sizeof(MY_TYPE);
	//	unsigned int startByte = count * oData->channels * sizeof(MY_TYPE);
	//	memset((char *)(outputBuffer)+startByte, 0, bytes);
		return 1;
	//}

	//return 0;



}

typedef struct {
	unsigned int	nRate;		/* Sampling Rate (sample/sec) */
	unsigned int	nChannel;	/* Channel Number */
	unsigned int	nFrame;		/* Frame Number of Wave Table */
	float		*wftable;	/* Wave Form Table(interleaved) */
	unsigned int	cur;		/* current index of WaveFormTable(in Frame) */
} CallbackData;

static int
rtaudio_callback(
	void			*outbuf,
	void			*inbuf,
	unsigned int		nFrames,
	double			streamtime,
	RtAudioStreamStatus	status,
	void			*userdata)
{
	(void)inbuf;
	float	*buf = (float*)outbuf;
	unsigned int remainFrames;
	CallbackData	*data = (CallbackData*)userdata;

	remainFrames = nFrames;
	while (remainFrames > 0) {
		unsigned int sz = data->nFrame - data->cur;
		if (sz > remainFrames)
			sz = remainFrames;
		memcpy(buf, data->wftable + (data->cur*data->nChannel),
			sz * data->nChannel * sizeof(float));
		data->cur = (data->cur + sz) % data->nFrame;
		buf += sz * data->nChannel;
		remainFrames -= sz;
	}
	return 0;
}

#define BASE_RATE 0.005
#define TIME   1.0
unsigned int frameCounter = 0;
bool checkCount = false;
unsigned int nFrames = 0;
const unsigned int callbackReturnValue = 1;
int saw(void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
	double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
	unsigned int i, j;
	unsigned int channels=2;
	MY_TYPE *buffer = (MY_TYPE *)outputBuffer;
	double *lastValues = (double *)data;

	if (status)
		std::cout << "Stream underflow detected!" << std::endl;

	double increment;
	for (j = 0; j < channels; j++) {
		increment = BASE_RATE * (j + 1 + (j*0.1));
		for (i = 0; i < nBufferFrames; i++) {
			*buffer++ = (MY_TYPE)(lastValues[j] * SCALE * 0.5);
			lastValues[j] += increment;
			if (lastValues[j] >= 1.0) lastValues[j] -= 2.0;
		}
	}

	frameCounter += nBufferFrames;
	if (checkCount && (frameCounter >= nFrames)) return callbackReturnValue;
	return 0;
}


void testlibsnd(double frequency, int offset, int duration);
void writeAudioFile2(char * fname, double *buffer, int bufferSize);



const int PLAYBACKFREQ = 44100;
const float PI2 = 3.14159265359f * 2;

void generateDTMF(short *buffer, int length, float freq1, float freq2)
{
	int i;
	short *dest = buffer;
	for (i = 0; i < length; i++)
	{
		*(dest++) = (sin(i*(PI2*(PLAYBACKFREQ / freq1))) + sin((PI2*(PLAYBACKFREQ / freq2)))) * 16383;
	}
}


short buffer2[100000];

void DTMFGen(double frequency1, double frequency2, int offSet, int durration);

void testReadAndFind(char *fname, double freq,int samples);
void testReadAndFind2(char *fname, double freq, int samples, int size);

int main( int argc, char *argv[] )
{
  unsigned int channels, fs, bufferFrames, device = 0, offset = 0;
  char *file;
  CallbackData data2;

  //generateDTMF(buffer2, 100000,)
  //testtone(samples);
  //writeAudioFile2("c:\\temp\\dtmf.wav", (double*)samples, 100000);
  DTMFGen(697, 1209, 0, 200000);
  //testlibsnd(440,0,200000);
  //13.5 inches per msec is how fast time travels 
  //1.3 inces per .1msec

  FILETIME ft_now;
  GetSystemTimeAsFileTime(&ft_now);

  long ll_now = (LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL);

  printf("start:%ld\n", ll_now);

  //testReadAndFind(R"(C:\Users\gbrill\Source\Repos\rtaudio-4.1.1\buildit\tests\sin.wav)",1209,1000000/sizeof(double));
  testReadAndFind2(R"(C:\Users\gbrill\Source\Repos\rtaudio-4.1.1\buildit\tests\sin.wav)", 1209, 1000 / sizeof(double),5000);
  
  GetSystemTimeAsFileTime(&ft_now);
  long ll_now2 = ll_now - (LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL);

  printf("time:%d %fmsec\n", ll_now,float(ll_now2)*1.0f/10000);

  return 0;

  // minimal command-line checking
  if ( argc < 4 || argc > 6 ) usage();

  RtAudio dac;
  if ( dac.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    exit( 0 );
  }

  channels = (unsigned int) atoi( argv[1]) ;
  fs = (unsigned int) atoi( argv[2] );
  file = argv[3];
  if ( argc > 4 )
    device = (unsigned int) atoi( argv[4] );
  if ( argc > 5 )
    offset = (unsigned int) atoi( argv[5] );

  OutputData data;
  data.fd = fopen( file, "rb" );
  if ( !data.fd ) {
    std::cout << "Unable to find or open file!\n";
   // exit( 1 );
  }

  // Set our stream parameters for output only.
  bufferFrames = 512;
  RtAudio::StreamParameters oParams;
  oParams.deviceId = device;
  oParams.nChannels = channels;
  oParams.firstChannel = offset;
  double data3;

  if ( device == 0 )
    oParams.deviceId = dac.getDefaultOutputDevice();

  data.channels = channels;
  try {
    dac.openStream( &oParams, NULL, FORMAT, fs, &bufferFrames, &output, (void *)&data ); //this opens and plays a file
	//  dac.openStream(&oParams, NULL, FORMAT, fs, &bufferFrames, &DTMF, (void *)samples);
	//  dac.openStream(&oParams, NULL, RTAUDIO_FLOAT32, 44100,  &bufferFrames, rtaudio_callback, &data2);
	 

    dac.startStream();
  }
  catch ( RtAudioError& e ) {
    std::cout << '\n' << e.getMessage() << '\n' << std::endl;
    goto cleanup;
  }

  std::cout << "\nPlaying raw file " << file << " (buffer frames = " << bufferFrames << ")." << std::endl;
  while ( 1 ) {
    SLEEP( 100 ); // wake every 100 ms to check if we're done
    if ( dac.isStreamRunning() == false ) break;
  }

 cleanup:
  fclose( data.fd );
  dac.closeStream();

  return 0;
}
