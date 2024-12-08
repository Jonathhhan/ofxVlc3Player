#include "ofxVlc3Player.h"

ofxVlcPlayer::ofxVlcPlayer()
	: libvlc(NULL)
	, eventManager(NULL)
	, media(NULL)
	, mediaPlayer(NULL)
	, ringBuffer(static_cast<size_t>(2048 * 2048)) {
	buffer.allocate(1, 2);
}

ofxVlcPlayer::~ofxVlcPlayer() { }

void ofxVlcPlayer::load(std::string name, int vlc_argc, char const * vlc_argv[]) {
	if (libvlc) {
		stop();
	}
	libvlc = libvlc_new(vlc_argc, vlc_argv);
	if (!libvlc) {
		const char * error = libvlc_errmsg();
		cout << error << endl;
		return;
	}
	if (ofStringTimesInString(name, "://") == 1) {
		media = libvlc_media_new_location(libvlc, name.c_str());
	} else {
		media = libvlc_media_new_path(libvlc, name.c_str());
	}
	mediaPlayer = libvlc_media_player_new_from_media(media);
	libvlc_media_parse(media);
	unsigned int x, y;
	if (libvlc_video_get_size(mediaPlayer, 0, &x, &y) != -1) {
		videoWidth = x;
		videoHeight = y;
	} else {
		videoWidth = 1280;
		videoHeight = 720;
	}
	std::cout << "video size: " << videoWidth << " * " << videoHeight << std::endl;
	std::cout << "media length in ms: " << libvlc_media_get_duration(media) << std::endl;

	libvlc_video_set_callbacks(mediaPlayer, lockStatic, NULL, NULL, this);
	libvlc_video_set_format(mediaPlayer, "RGBA", videoWidth, videoHeight, videoWidth * 4);

	libvlc_audio_set_callbacks(mediaPlayer, audioPlay, audioPause, audioResume, audioFlush, audioDrain, this);
	libvlc_audio_set_format_callbacks(mediaPlayer, audioSetup, audioCleanup);

	eventManager = libvlc_media_player_event_manager(mediaPlayer);
	libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, vlcEventStatic, this);

	image.allocate(videoWidth, videoHeight, OF_IMAGE_COLOR_ALPHA);
}

void ofxVlcPlayer::audioPlay(void * data, const void * samples, unsigned int count, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	that->isAudioReady = true;
	short * sampleArray = (short *)samples;
	float sampleArrayConverted[10000] {};
	for (int i = 0; i < count * that->channels; i++) {
		sampleArrayConverted[i] = ofMap(sampleArray[i], -32768, 32768, -1, 1);
	}
	that->buffer.copyFrom(sampleArrayConverted, count, that->channels, that->sampleRate);
	that->ringBuffer.writeFromBuffer(that->buffer);
	// std::cout << "sample size : " << sampleArrayConverted[0] << ", pts: " << pts << std::endl;
	// std::cout << "readable samples: " << that->ringBuffer.getNumReadableSamples() << ", read position: " << that->ringBuffer.getReadPosition() << std::endl;
}

void ofxVlcPlayer::audioPause(void * data, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	that->isAudioReady = false;
	// std::cout << "audio pause" << std::endl;
}

void ofxVlcPlayer::audioResume(void * data, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	// std::cout << "audio resume" << std::endl;
}

void ofxVlcPlayer::audioFlush(void * data, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	// std::cout << "audio flush" << std::endl;
}

void ofxVlcPlayer::audioDrain(void * data) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	// std::cout << "audio drain " << std::endl;
}

int ofxVlcPlayer::audioSetup(void ** data, char * format, unsigned int * rate, unsigned int * channels) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(*data);
	strncpy(format, "S16N", 4);
	that->sampleRate = rate[0];
	that->channels = channels[0];
	that->ringBuffer._readStart = 0;
	that->ringBuffer._writeStart = 0;
	std::cout << "audio format : " << format << ", rate: " << rate[0] << ", channels: " << channels[0] << std::endl;
	return 0;
}

void ofxVlcPlayer::audioCleanup(void * data) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	that->isAudioReady = false;
	std::cout << "audio cleanup" << std::endl;
}

void ofxVlcPlayer::update() {
	image.update();
}

ofTexture & ofxVlcPlayer::getTexture() {
	return image.getTexture();
}

void ofxVlcPlayer::draw(float x, float y, float w, float h) {
	image.draw(x, y, w, h);
}

void ofxVlcPlayer::draw(float x, float y) {
	getTexture().draw(x, y);
}

void ofxVlcPlayer::play() {
	libvlc_media_player_play(mediaPlayer);
}

void ofxVlcPlayer::pause() {
	libvlc_media_player_pause(mediaPlayer);
}

void ofxVlcPlayer::stop() {
	libvlc_media_player_stop(mediaPlayer);
}

void ofxVlcPlayer::setPosition(float pct) {
	libvlc_media_player_set_position(mediaPlayer, pct);
}

void ofxVlcPlayer::setLoop(bool loop) {
	isLooping = loop;
}

bool ofxVlcPlayer::getLoop() const {
	return isLooping;
}

float ofxVlcPlayer::getHeight() const {
	return videoHeight;
}

float ofxVlcPlayer::getWidth() const {
	return videoWidth;
}

bool ofxVlcPlayer::isPlaying() {
	return libvlc_media_player_is_playing(mediaPlayer);
}

bool ofxVlcPlayer::isSeekable() {
	return libvlc_media_player_is_seekable(mediaPlayer);
}

float ofxVlcPlayer::getPosition() {
	return libvlc_media_player_get_position(mediaPlayer);
}

int ofxVlcPlayer::getTime() {
	return libvlc_media_player_get_time(mediaPlayer);
}

void ofxVlcPlayer::setTime(int ms) {
	libvlc_media_player_set_time(mediaPlayer, ms);
}

float ofxVlcPlayer::getFps() {
	return libvlc_media_player_get_fps(mediaPlayer);
}

void ofxVlcPlayer::setFrame(int frame) {
	libvlc_media_player_set_time(mediaPlayer, 1000 * frame / libvlc_media_player_get_fps(mediaPlayer));
}

int ofxVlcPlayer::getCurrentFrame() {
	return libvlc_media_player_get_fps(mediaPlayer) * libvlc_media_player_get_time(mediaPlayer) / 1000;
}

int ofxVlcPlayer::getTotalNumFrames() {
	return libvlc_media_player_get_fps(mediaPlayer) * libvlc_media_player_get_length(mediaPlayer) / 1000;
}

float ofxVlcPlayer::getLength() {
	return libvlc_media_player_get_length(mediaPlayer);
}

void ofxVlcPlayer::setVolume(int volume) {
	libvlc_audio_set_volume(mediaPlayer, volume);
}

void ofxVlcPlayer::toggleMute() {
	libvlc_audio_toggle_mute(mediaPlayer);
}

void * ofxVlcPlayer::lockStatic(void * data, void ** p_pixels) {
	return ((ofxVlcPlayer *)data)->lock(p_pixels);
}

void ofxVlcPlayer::vlcEventStatic(const libvlc_event_t * event, void * data) {
	((ofxVlcPlayer *)data)->vlcEvent(event);
}

void ofxVlcPlayer::vlcEvent(const libvlc_event_t * event) {
	if (event->type == libvlc_MediaPlayerEndReached) {
		if (isLooping) {
			mediaPlayer = libvlc_media_player_new_from_media(media);
			libvlc_video_set_callbacks(mediaPlayer, lockStatic, NULL, NULL, this);
			libvlc_video_set_format(mediaPlayer, "RGBA", videoWidth, videoHeight, videoWidth * 4);
			eventManager = libvlc_media_player_event_manager(mediaPlayer);
			libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, vlcEventStatic, this);
			play();
		}
	}
}

void * ofxVlcPlayer::lock(void ** p_pixels) {
	*p_pixels = image.getPixels().getData();
	return NULL;
}

void ofxVlcPlayer::close() {
	libvlc_media_player_release(mediaPlayer);
	libvlc_media_release(media);
	libvlc_free(libvlc);
}

bool ofxVlcPlayer::audioIsReady() {
	return isAudioReady;
}
