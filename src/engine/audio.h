#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

// 音频通道类型
enum class AudioChannel {
    BGM,    // 背景音乐
    VOICE,  // 语音
    SE,     // 音效
    SYSTEM  // 系统音效
};

// 音频系统类
class Audio {
public:
    Audio();
    ~Audio();

    // BGM 播放
    void playBGM(const std::string& path, bool loop = true, int fadeMs = 0);
    void stopBGM(int fadeMs = 0);
    void pauseBGM();
    void resumeBGM();
    bool isBGMPlaying() const;

    // 语音播放
    void playVoice(const std::string& path);
    void stopVoice();

    // 音效播放
    void playSE(const std::string& path, int channel = -1);
    void stopAllSE();

    // 音量控制
    void setMasterVolume(int vol);  // 0-128
    void setBGMVolume(int vol);
    void setVoiceVolume(int vol);
    void setSEVolume(int vol);
    int getBGMVolume() const { return m_bgmVolume; }
    int getVoiceVolume() const { return m_voiceVolume; }
    int getSEVolume() const { return m_seVolume; }

    // 更新（处理淡入淡出）
    void update(float dt);

private:
    Mix_Music* m_bgm = nullptr;
    std::string m_bgmPath;
    bool m_bgmPlaying = false;

    int m_masterVolume = 128;
    int m_bgmVolume = 96;
    int m_voiceVolume = 128;
    int m_seVolume = 96;

    // 音效缓存
    std::unordered_map<std::string, Mix_Chunk*> m_seCache;

    Mix_Chunk* loadSE(const std::string& path);
};
