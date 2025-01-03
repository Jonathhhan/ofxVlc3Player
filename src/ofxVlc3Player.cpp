#include "ofxVlc3Player.h"

ofxVlcPlayer::ofxVlcPlayer()
	: libvlc(NULL)
	, mediaPlayerEventManager(NULL)
	, mediaEventManager(NULL)
	, media(NULL)
	, mediaPlayer(NULL)
	, ringBuffer(static_cast<size_t>(50000)) {
	buffer.allocate(1, 2);
	image.allocate(1, 1, OF_IMAGE_COLOR_ALPHA);
}

ofxVlcPlayer::~ofxVlcPlayer() { }

void ofxVlcPlayer::init(int vlc_argc, char const * vlc_argv[]) {
	libvlc = libvlc_new(vlc_argc, vlc_argv);
	if (!libvlc) {
		const char * error = libvlc_errmsg();
		cout << error << endl;
		return;
	}
	mediaPlayer = libvlc_media_player_new(libvlc);
	libvlc_video_set_callbacks(mediaPlayer, lockStatic, NULL, NULL, this);
	libvlc_video_set_format_callbacks(mediaPlayer, videoFormat, videoCleanup);

	libvlc_audio_set_callbacks(mediaPlayer, audioPlay, audioPause, audioResume, audioFlush, audioDrain, this);
	libvlc_audio_set_format_callbacks(mediaPlayer, audioSetup, audioCleanup);

	mediaPlayerEventManager = libvlc_media_player_event_manager(mediaPlayer);
	libvlc_event_attach(mediaPlayerEventManager, libvlc_MediaPlayerEndReached, vlcMediaPlayerEventStatic, this);
}

void ofxVlcPlayer::load(std::string name) {
	if (!libvlc) {
		std::cout << "initialize libvlc first!" << std::endl;
	} else {
		if (ofStringTimesInString(name, "://") == 1) {
			media = libvlc_media_new_location(libvlc, name.c_str());
		} else {
			media = libvlc_media_new_path(libvlc, name.c_str());
		}
		mediaEventManager = libvlc_media_event_manager(media);
		libvlc_event_attach(mediaEventManager, libvlc_MediaParsedChanged, vlcMediaEventStatic, this);
		libvlc_media_parse(media);
		libvlc_media_player_set_media(mediaPlayer, media);
		if (libvlc_video_get_size(mediaPlayer, 0, &videoWidth, &videoHeight) != -1) {
			image.clear();
			image.allocate(videoWidth, videoHeight, OF_IMAGE_COLOR_ALPHA);
			image.getPixels().swapRgb();
			std::cout << "video size: " << videoWidth << " * " << videoHeight << std::endl;
		}
		// libvlc_media_player_record(mediaPlayer, true, &ofToDataPath("")[0]);
	}
}

unsigned int ofxVlcPlayer::videoFormat(void ** data, char * chroma, unsigned int * width, unsigned int * height, unsigned int * pitches, unsigned int * lines) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(*data);
	// std::cout << "chroma: " << chroma << " " << lines[0] << " " << pitches[0] << " " << width[0] << " " << height[0] << std::endl;
	strncpy(chroma, "RV32", 4);
	pitches[0] = width[0] * 4;
	lines[0] = that->videoHeight;
	return 1;
}

void ofxVlcPlayer::videoCleanup(void * data) {
	// std::cout << "video cleanup" << std::endl;
}

void ofxVlcPlayer::audioPlay(void * data, const void * samples, unsigned int count, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	that->isAudioReady = true;
	that->buffer.copyFrom((short *)samples, count, that->channels, that->sampleRate);
	that->ringBuffer.writeFromBuffer(that->buffer);
	// std::cout << "sample size : " << sampleArrayConverted[0] << ", pts: " << pts << std::endl;
	// std::cout << "readable samples: " << that->ringBuffer.getNumReadableSamples() << ", read position: " << that->ringBuffer.getReadPosition() << std::endl;
}

void ofxVlcPlayer::audioPause(void * data, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	that->isAudioReady = false;
	std::cout << "audio pause" << std::endl;
}

void ofxVlcPlayer::audioResume(void * data, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	std::cout << "audio resume" << std::endl;
}

void ofxVlcPlayer::audioFlush(void * data, int64_t pts) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	std::cout << "audio flush" << std::endl;
}

void ofxVlcPlayer::audioDrain(void * data) {
	ofxVlcPlayer * that = static_cast<ofxVlcPlayer *>(data);
	std::cout << "audio drain " << std::endl;
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

void ofxVlcPlayer::vlcMediaPlayerEventStatic(const libvlc_event_t * event, void * data) {
	((ofxVlcPlayer *)data)->vlcMediaPlayerEvent(event);
}

void ofxVlcPlayer::vlcMediaPlayerEvent(const libvlc_event_t * event) {
	if (event->type == libvlc_MediaPlayerEndReached) {
		if (isLooping) {
			mediaPlayer = libvlc_media_player_new_from_media(media);
			libvlc_video_set_callbacks(mediaPlayer, lockStatic, NULL, NULL, this);
			libvlc_video_set_format_callbacks(mediaPlayer, videoFormat, videoCleanup);
			libvlc_audio_set_callbacks(mediaPlayer, audioPlay, audioPause, audioResume, audioFlush, audioDrain, this);
			libvlc_audio_set_format_callbacks(mediaPlayer, audioSetup, audioCleanup);
			mediaPlayerEventManager = libvlc_media_player_event_manager(mediaPlayer);
			libvlc_event_attach(mediaPlayerEventManager, libvlc_MediaPlayerEndReached, vlcMediaPlayerEventStatic, this);
			play();
		}
	}
}

void ofxVlcPlayer::vlcMediaEventStatic(const libvlc_event_t * event, void * data) {
	((ofxVlcPlayer *)data)->vlcMediaEvent(event);
}

void ofxVlcPlayer::vlcMediaEvent(const libvlc_event_t * event) {
	if (event->type == libvlc_MediaParsedChanged) {
		std::cout << "media length in ms: " << libvlc_media_get_duration(media) << std::endl;
	}
}

void * ofxVlcPlayer::lockStatic(void * data, void ** p_pixels) {
	return ((ofxVlcPlayer *)data)->lock(p_pixels);
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
