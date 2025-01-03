#pragma once

#include "LockFreeRingBuffer.h"
#include "ofMain.h"
#include "vlc/vlc.h"

class ofxVlcPlayer {
	ofImage image;
	ofSoundBuffer buffer;

	libvlc_instance_t * libvlc;
	libvlc_media_t * media;
	libvlc_media_player_t * mediaPlayer;
	libvlc_event_manager_t * mediaPlayerEventManager;
	libvlc_event_manager_t * mediaEventManager;

	int ringBufferSize = 0;
	int channels = 0;
	int sampleRate = 0;
	bool isLooping = false;
	bool isAudioReady = false;

	// VLC Video callbaks
	static void * lockStatic(void * data, void ** p_pixels);
	void * lock(void ** p_pixels);
	static unsigned int videoFormat(void ** data, char * chroma, unsigned int * width, unsigned int * height, unsigned int * pitches, unsigned int * lines);
	static void videoCleanup(void * data);

	// VLC Audio callbaks
	static void audioPlay(void * data, const void * samples, unsigned int count, int64_t pts);
	static void audioPause(void * data, int64_t pts);
	static void audioResume(void * data, int64_t pts);
	static void audioFlush(void * data, int64_t pts);
	static void audioDrain(void * data);

	static int audioSetup(void ** data, char * format, unsigned int * rate, unsigned int * channels);
	static void audioCleanup(void * data);

	// VLC Event callbacks
	static void vlcMediaPlayerEventStatic(const libvlc_event_t * event, void * data);
	void vlcMediaPlayerEvent(const libvlc_event_t * event);
	static void vlcMediaEventStatic(const libvlc_event_t * event, void * data);
	void vlcMediaEvent(const libvlc_event_t * event);

public:
	ofxVlcPlayer();
	virtual ~ofxVlcPlayer();
	void init(int vlc_argc, char const * vlc_argv[]);
	void load(std::string name);
	void update();
	ofTexture & getTexture();
	void draw(float x, float y, float w, float h);
	void draw(float x, float y);
	void play();
	void pause();
	void stop();
	void setPosition(float pct);
	void setLoop(bool loop);
	bool getLoop() const;
	float getHeight() const;
	float getWidth() const;
	bool isPlaying();
	bool isSeekable();
	float getPosition();
	int getTime();
	void setTime(int ms);
	float getFps();
	float getLength();
	void setFrame(int frame);
	int getCurrentFrame();
	int getTotalNumFrames();
	void setVolume(int volume);
	void toggleMute();
	void close();
	bool audioIsReady();
	LockFreeRingBuffer ringBuffer;
	unsigned int videoWidth = 0;
	unsigned int videoHeight = 0;
};
