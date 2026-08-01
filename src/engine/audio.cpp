#include "audio.h"
#include <iostream>

Audio::Audio() {}

Audio::~Audio() {
    stopBGM();
    stopAllSE();
    for (auto& [path, chunk] : m_seCache) {
        if (chunk) Mix_FreeChunk(chunk);
    }
    m_seCache.clear();
}

void Audio::playBGM(const std::string& path, bool loop, int fadeMs) {
    if (m_bgmPath == path && m_bgmPlaying) return; // 同一首 BGM 不重复播放

    stopBGM(fadeMs > 0 ? fadeMs : 0);

    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music) {
        std::cerr << "Failed to load BGM: " << path << " - " << Mix_GetError() << std::endl;
        return;
    }
    m_bgm = music;
    m_bgmPath = path;
    Mix_VolumeMusic(m_bgmVolume * m_masterVolume / 128);

    if (fadeMs > 0) {
        Mix_FadeInMusic(m_bgm, -1, fadeMs);
    } else {
        Mix_PlayMusic(m_bgm, loop ? -1 : 0);
    }
    m_bgmPlaying = true;
}

void Audio::stopBGM(int fadeMs) {
    if (!m_bgmPlaying) return;
    if (fadeMs > 0) {
        Mix_FadeOutMusic(fadeMs);
    } else {
        Mix_HaltMusic();
    }
    if (m_bgm) {
        Mix_FreeMusic(m_bgm);
        m_bgm = nullptr;
    }
    m_bgmPlaying = false;
    m_bgmPath.clear();
}

void Audio::pauseBGM() { Mix_PauseMusic(); }
void Audio::resumeBGM() { Mix_ResumeMusic(); }
bool Audio::isBGMPlaying() const { return Mix_PlayingMusic() && !Mix_PausedMusic(); }

void Audio::playVoice(const std::string& path) {
    Mix_Chunk* voice = Mix_LoadWAV(path.c_str());
    if (!voice) {
        std::cerr << "Failed to load voice: " << path << " - " << Mix_GetError() << std::endl;
        return;
    }
    Mix_VolumeChunk(voice, m_voiceVolume * m_masterVolume / 128);
    // 使用通道 0 播放语音（可被新语音中断）
    Mix_HaltChannel(0);
    Mix_PlayChannel(0, voice, 0);
    // 注意：需要设置 channel finished callback 来释放
    Mix_ChannelFinished([](int channel) {
        // 简化处理：语音播完自动释放
    });
}

void Audio::stopVoice() { Mix_HaltChannel(0); }

Mix_Chunk* Audio::loadSE(const std::string& path) {
    auto it = m_seCache.find(path);
    if (it != m_seCache.end()) return it->second;

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        std::cerr << "Failed to load SE: " << path << " - " << Mix_GetError() << std::endl;
        return nullptr;
    }
    m_seCache[path] = chunk;
    return chunk;
}

void Audio::playSE(const std::string& path, int channel) {
    Mix_Chunk* chunk = loadSE(path);
    if (!chunk) return;
    Mix_VolumeChunk(chunk, m_seVolume * m_masterVolume / 128);
    Mix_PlayChannel(channel, chunk, 0);
}

void Audio::stopAllSE() {
    // 只停止音效通道（1-31），保留语音通道（0）
    for (int i = 1; i < 32; i++) Mix_HaltChannel(i);
}

void Audio::setMasterVolume(int vol) {
    m_masterVolume = std::max(0, std::min(128, vol));
    Mix_VolumeMusic(m_bgmVolume * m_masterVolume / 128);
}

void Audio::setBGMVolume(int vol) {
    m_bgmVolume = std::max(0, std::min(128, vol));
    Mix_VolumeMusic(m_bgmVolume * m_masterVolume / 128);
}

void Audio::setVoiceVolume(int vol) {
    m_voiceVolume = std::max(0, std::min(128, vol));
}

void Audio::setSEVolume(int vol) {
    m_seVolume = std::max(0, std::min(128, vol));
}

void Audio::update(float dt) {
    // 淡入淡出由 SDL_mixer 内部处理
    // 这里可扩展自定义淡入淡出逻辑
}
