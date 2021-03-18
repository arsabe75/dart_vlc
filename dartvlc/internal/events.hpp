/*
 * dart_vlc: A media playback library for Dart & Flutter. Based on libVLC & libVLC++.
 * 
 * Hitesh Kumar Saini
 * https://github.com/alexmercerind
 * alexmercerind@gmail.com
 * 
 * GNU Lesser General Public License v2.1
 */

#include "getters.hpp"


class PlayerEvents : public PlayerGetters {
public:
	void onOpen(std::function<void(VLC::Media)> callback) {
		this->_openCallback = callback;
		this->mediaPlayer.eventManager().onMediaChanged(
			std::bind(&PlayerEvents::_onOpenCallback, this, std::placeholders::_1)
		);
	}

	void onPlay(std::function<void(void)> callback) {
		this->_playCallback = callback;
		this->mediaPlayer.eventManager().onPlaying(
			std::bind(&PlayerEvents::_onPlayCallback, this)
		);
	}

	void onPause(std::function<void(void)> callback) {
		this->_pauseCallback = callback;
		this->mediaPlayer.eventManager().onPaused(
			std::bind(&PlayerEvents::_onPauseCallback, this)
		);
	}

	void onStop(std::function<void(void)> callback) {
		this->_stopCallback = callback;
		this->mediaPlayer.eventManager().onStopped(
			std::bind(&PlayerEvents::_onStopCallback, this)
		);
	}

	void onPosition(std::function<void(int)> callback) {
		this->_positionCallback = callback;
		this->mediaPlayer.eventManager().onPositionChanged(
			std::bind(&PlayerEvents::_onPositionCallback, this, std::placeholders::_1)
		);
	}

	void onSeekable(std::function<void(bool)> callback) {
		this->_seekableCallback = callback;
		this->mediaPlayer.eventManager().onSeekableChanged(
			std::bind(&PlayerEvents::_onSeekableCallback, this, std::placeholders::_1)
		);
	}

	void onComplete(std::function<void(void)> callback) {
		this->_completeCallback = callback;
		this->mediaPlayer.eventManager().onEndReached(
			std::bind(&PlayerEvents::_onCompleteCallback, this)
		);
	}

	void onVolume(std::function<void(float)> callback) {
		this->_volumeCallback = callback;
	}

	void onRate(std::function<void(float)> callback) {
		this->_rateCallback = callback;
	}

	void onPlaylist(std::function<void(void)> callback) {
		this->_playlistCallback = callback;
	}

protected:

	std::function<void(void)> _playlistCallback;

	/*
	 * Related to `Playlist` modifications by `this->add`, `this->remove` and `this->insert`.
	 * This only gets called when there is change in currently playing `Media` e.g. by `this->next` or `this->back` or on completion of the playback.
	 * This method is called to update `Media` in the `Playlist` without user noticing.
	 * 
	 * Pass `false` for silently updating playlist & `true` to play the resulting current `Media`.
	 */
	void _onPlaylistCallback(bool play = false) {
		/* Check if `this->mediaList` is modified by `this->add`, `this->remove` or `this->insert`. */
		if (this->isPlaylistModified) {
			this->mediaListPlayer.setMediaList(this->mediaList);
			/* Stop the `Player` if the `this->mediaList` is empty after any `Playlist` modifications. */
			if (!this->mediaList.count()) {
				this->state = new PlayerState();
				this->mediaListPlayer.stop();
				return;
			}
			/* Set the `this->state->index` to end if it exceeds the length. */
			if (this->state->index > this->mediaList.count())
				this->state->index = this->mediaList.count() - 1;
			if (play)
				this->mediaListPlayer.playItemAtIndex(this->state->index);
			this->isPlaylistModified = false;
			this->_playlistCallback();
		};
	}

	std::function<void(VLC::Media)> _openCallback;

	void _onOpenCallback(VLC::MediaPtr media) {
		this->state->isPlaying = this->mediaPlayer.isPlaying();
		this->state->isValid = this->mediaPlayer.isValid();
		if (this->getDuration() > 0) {
			this->state->isCompleted = false;
			this->state->position = this->getPosition();
			this->state->duration = this->getDuration();
		}
		else {
			this->state->isCompleted = false;
			this->state->position = 0;
			this->state->duration = 0;
		}
		this->state->index = this->mediaList.indexOfItem(*media.get());
		this->_openCallback(*media.get());
	}

	std::function<void(void)> _playCallback;

	void _onPlayCallback() {
		if (this->getDuration() > 0) {
			this->state->isPlaying = this->mediaPlayer.isPlaying();
			this->state->isValid = this->mediaPlayer.isValid();
			this->state->isCompleted = false;
			this->state->position = this->getPosition();
			this->state->duration = this->getDuration();
			this->_playCallback();
		}
	}

	std::function<void(void)> _pauseCallback;

	void _onPauseCallback() {
		if (this->getDuration() > 0) {
			this->state->isPlaying = this->mediaPlayer.isPlaying();
			this->state->isValid = this->mediaPlayer.isValid();
			this->state->position = this->getPosition();
			this->state->duration = this->getDuration();
			this->_pauseCallback();
		}
	}

	std::function<void(void)> _stopCallback;

	void _onStopCallback() {
		if (this->getDuration() > 0) {
			this->state->isPlaying = this->mediaPlayer.isPlaying();
			this->state->isValid = this->mediaPlayer.isValid();
			this->state->position = this->getPosition();
			this->state->duration = this->getDuration();
			this->_stopCallback();
		}
	}

	std::function<void(int)> _positionCallback;

	void _onPositionCallback(float relativePosition) {
		if (this->getDuration() > 0) {
			this->state->isPlaying = this->mediaPlayer.isPlaying();
			this->state->isValid = this->mediaPlayer.isValid();
			this->state->position = this->getPosition();
			this->state->duration = this->getDuration();
			this->_positionCallback(
				static_cast<int>(relativePosition * this->mediaPlayer.length())
			);
		}
	}

	std::function<void(bool)> _seekableCallback;

	void _onSeekableCallback(bool isSeekable) {
		if (this->getDuration() > 0) {
			this->state->isSeekable = isSeekable;
			this->_seekableCallback(isSeekable);
		}
	}

	std::function<void(void)> _completeCallback;

	void _onCompleteCallback() {
		if (this->getDuration() > 0) {
			this->state->isPlaying = this->mediaPlayer.isPlaying();
			this->state->isValid = this->mediaPlayer.isValid();
			this->state->isCompleted = true;
			this->state->position = this->getPosition();
			this->state->duration = this->getDuration();
			/* Explicitly change current `Media` & play it since playback is ended. */
			this->_onPlaylistCallback(true);
			this->_completeCallback();
		}
	}

	std::function<void(float)> _volumeCallback;

	std::function<void(float)> _rateCallback;
};