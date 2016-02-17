#include <math.h>

#include <RtAudio.h>
//doesnt seem to work at all
#if defined(__cplusplus)
extern "C" {
#endif

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

	int
		playsin(void)
	{
		RtAudio *audio;
		unsigned int bufsize = 4096;
		CallbackData data;

		try {
			audio = new RtAudio(RtAudio::WINDOWS_WASAPI);
		}
		catch (...) {
			
			return 1;
		}
		if (!audio) {
			fprintf(stderr, "fail to allocate RtAudio¥n");
			return 1;
		}
		/* probe audio devices */
		unsigned int devId = audio->getDefaultOutputDevice();

		/* Setup output stream parameters */
		RtAudio::StreamParameters *outParam = new RtAudio::StreamParameters();

		outParam->deviceId = devId;
		outParam->nChannels = 2;

		audio->openStream(outParam, NULL, RTAUDIO_FLOAT32, 44100,
			&bufsize, rtaudio_callback, &data);

		/* Create Wave Form Table */
		data.nRate = 44100;
		/* Frame Number is based on Freq(440Hz) and Sampling Rate(44100) */
		/* hmm... nFrame = 44100 is enough approximation, maybe... */
		data.nFrame = 44100;
		data.nChannel = outParam->nChannels;
		data.cur = 0;
		data.wftable = (float *)calloc(data.nChannel * data.nFrame, sizeof(float));
		if (!data.wftable)
		{
			delete audio;
			fprintf(stderr, "fail to allocate memory¥n");
			return 1;
		}
		for (unsigned int i = 0; i < data.nFrame; i++) {
			float v = sin(i * 3.1416 * 2 * 440 / data.nRate);
			for (unsigned int j = 0; j < data.nChannel; j++) {
				data.wftable[i*data.nChannel + j] = v;
			}
		}

		audio->startStream();
//		sleep(10);
		audio->stopStream();
		audio->closeStream();
		delete audio;

		return 0;
	}

#if defined(__cplusplus)
}
#endif