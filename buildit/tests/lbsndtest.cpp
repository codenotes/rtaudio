#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sndfile.h>

#define BUFFER_SIZE 1000000
#define SAMPLE_RATE 44100.0
#define PI 3.141592654

void writeAudioFile(double *buffer) 
{

	SF_INFO sndInfo;

	sndInfo.samplerate = SAMPLE_RATE;
	sndInfo.channels = 1;
	sndInfo.frames = BUFFER_SIZE;
	sndInfo.format = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;

	SNDFILE *outFile = sf_open("sin.wav", SFM_WRITE, &sndInfo);

	sf_writef_double(outFile, buffer, BUFFER_SIZE);
	sf_close(outFile);
}


void writeAudioFile2(char * fname, double *buffer, int bufferSize=BUFFER_SIZE)
{

	SF_INFO sndInfo;

	sndInfo.samplerate = SAMPLE_RATE;
	sndInfo.channels = 1;
	sndInfo.frames = BUFFER_SIZE;
	sndInfo.format = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;

	SNDFILE *outFile = sf_open(fname, SFM_WRITE, &sndInfo);

	sf_writef_double(outFile, buffer, BUFFER_SIZE);
	sf_close(outFile);
}

void readFile(char *fname, double * out, int size)
{

	SF_INFO sndInfo;

	//sndInfo.samplerate = SAMPLE_RATE;
	//sndInfo.channels = 1;
	//sndInfo.frames = BUFFER_SIZE;
	//sndInfo.format = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;

	SNDFILE *inFile = sf_open(fname, SFM_READ, &sndInfo);
	sf_readf_double(inFile, out, size);


}
//http://wiki.laptop.org/go/Distance

float goertzel_mag(int numSamples, int TARGET_FREQUENCY, int SAMPLING_RATE, double* data)
{
	int     k, i;
	float   floatnumSamples;
	float   omega, sine, cosine, coeff, q0, q1, q2, magnitude, real, imag;

	float   scalingFactor = numSamples / 2.0;

	floatnumSamples = (float)numSamples;
	k = (int)(0.5 + ((floatnumSamples * TARGET_FREQUENCY) / SAMPLING_RATE));
	omega = (2.0 * PI * k) / floatnumSamples;
	sine = sin(omega);
	cosine = cos(omega);
	coeff = 2.0 * cosine;
	q0 = 0;
	q1 = 0;
	q2 = 0;

	for (i = 0; i < numSamples; i++)
	{
		q0 = coeff * q1 - q2 + data[i];
		q2 = q1;
		q1 = q0;
	}

	// calculate the real and imaginary results
	// scaling appropriately
	real = (q1 - q2 * cosine) / scalingFactor;
	imag = (q2 * sine) / scalingFactor;

	magnitude = sqrtf(real*real + imag*imag);
	return magnitude;
}


double buffer[BUFFER_SIZE];
double buffer2[BUFFER_SIZE];


void testReadAndFind(char *fname, double freq, int samples)
{

	readFile(fname, buffer2, BUFFER_SIZE);
	float mag=goertzel_mag(samples, freq, SAMPLE_RATE, buffer2);


	printf("mag:%f\n", mag);




}


void testReadAndFind2(char *fname, double freq, int samples, int size)
{

	readFile(fname, buffer2, size);
	float mag = goertzel_mag(samples, freq, SAMPLE_RATE, buffer2);


	printf("mag:%f\n", mag);




}


void DTMFGen(double frequency1,double frequency2, int offSet, int durration)
{

		for (int s = 0; s < BUFFER_SIZE; ++s)
			buffer[s] = 0;

		// sine wave: y(t) = amplitude * sin(2 * PI * frequency * time), time = s / sample_rate

		/*

		DTMF keypad frequencies (with sound clips)
			1209  1336 1477 1633 Hz
		697 Hz	1	2	3	A
		770 Hz	4	5	6	B
		852 Hz	7	8	9	C
		941 Hz	*	0	#	D
		Decoding[edit]

		*/

		//	double frequency;
		//	int offSet;
		//	int durration;

	//	printf("\nSin Wave Generator\n\n");
		//while (scanf("%lf %d %d", &frequency, &offSet, &durration) == 3) {

		double amplitude = 1.0;

		//printf("%lf Hz\t%d offset\t%d durration\n", frequency, offSet, durration);

		for (int s = offSet; s < BUFFER_SIZE && s < offSet + durration; ++s)
		{
			buffer[s] += amplitude * sin((2.0 * PI * frequency1) * (s / SAMPLE_RATE));
			buffer[s] += amplitude * sin((2.0 * PI * frequency2) * (s / SAMPLE_RATE));
			//add other tones here
			//buffer[s] += amplitude * sin((2.0 * PI * frequency*1.3) * (s / SAMPLE_RATE));
		}
		//}

		double max = 1.0;
		for (int s = 0; s < BUFFER_SIZE; ++s)
			if (buffer[s] > max)
				max = buffer[s];

		//normalize
		for (int s = 0; s < BUFFER_SIZE; ++s)
			buffer[s] /= max;

		writeAudioFile(buffer);
	

}

void testlibsnd(double frequency,int offSet,int durration) 
{
	

	for (int s = 0; s < BUFFER_SIZE; ++s)
		buffer[s] = 0;

	// sine wave: y(t) = amplitude * sin(2 * PI * frequency * time), time = s / sample_rate

	/*
	A  : 440
	A# : 466.16
	B  : 493.88
	C  : 523.25
	C# : 554.37
	D  : 587.33
	D# : 622.25
	E  : 659.26
	F  : 698.46
	F# : 739.99
	G  : 783.99
	G# : 830.61
	*/

//	double frequency;
//	int offSet;
//	int durration;

	printf("\nSin Wave Generator\n\n");
	//while (scanf("%lf %d %d", &frequency, &offSet, &durration) == 3) {

		double amplitude = 1.0;

		printf("%lf Hz\t%d offset\t%d durration\n", frequency, offSet, durration);

		for (int s = offSet; s < BUFFER_SIZE && s < offSet + durration; ++s)
		{
			buffer[s] += amplitude * sin((2.0 * PI * frequency) * (s / SAMPLE_RATE));
			//add other tones here
			//buffer[s] += amplitude * sin((2.0 * PI * frequency*1.3) * (s / SAMPLE_RATE));
		}
	//}

	double max = 1.0;
	for (int s = 0; s < BUFFER_SIZE; ++s)
		if (buffer[s] > max)
			max = buffer[s];

	//normalize
	for (int s = 0; s < BUFFER_SIZE; ++s)
		buffer[s] /= max;

	writeAudioFile(buffer);
}