#pragma once

#include "ofMain.h"
#include "vlc/vlc.h"
#include "LockFreeRingBuffer.h"

class ofxVlcPlayer {
    ofImage image;

    libvlc_instance_t* libvlc;
    libvlc_media_t* media;
    libvlc_media_player_t* mediaPlayer;
    libvlc_event_manager_t* eventManager;

    int videoWidth = 0;
    int videoHeight = 0;
    int ringBufferSize = 0;
    int channels = 0;
    int sampleRate = 0;
    bool isLooping = false;
    bool isAudioReady = false;

    // VLC Video callbaks
    static void* lockStatic(void* data, void** p_pixels);
    void* lock(void** p_pixels);

    // VLC Audio callbaks
    static void audioPlay(void* data, const void* samples, unsigned int count, int64_t pts);
    static void audioPause(void* data, int64_t pts);
    static void audioResume(void* data, int64_t pts);
    static void audioFlush(void* data, int64_t pts);
    static void audioDrain(void* data);

    static int audioSetup(void** data, char* format, unsigned int* rate, unsigned int* channels);
    static void audioCleanup(void* data);

    // VLC Event callbacks
    static void vlcEventStatic(const libvlc_event_t* event, void* data);
    void vlcEvent(const libvlc_event_t* event);

public:
    ofxVlcPlayer();
    virtual ~ofxVlcPlayer();
    void load(std::string name, int vlc_argc, char const* vlc_argv[]);
    void update();
    ofTexture& getTexture();
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
    ofSoundBuffer buffer;
};
