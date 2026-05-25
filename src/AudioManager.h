#pragma once
#include "raylib.h"
#include <unordered_map>
#include <string>

class AudioManager {
public:
    // Load sound (cached)
    static Sound LoadSound(const std::string &path) {
        auto it = sounds.find(path);
        if (it != sounds.end()) return it->second;
        Sound s = ::LoadSound(path.c_str());
        sounds[path] = s;
        return s;
    }
    // Load music stream (cached)
    static Music LoadMusic(const std::string &path) {
        auto it = musics.find(path);
        if (it != musics.end()) return it->second;
        Music m = ::LoadMusicStream(path.c_str());
        musics[path] = m;
        return m;
    }
    static void PlayMusic(Music &m, float volume=1.0f) {
        SetMusicVolume(m, volume);
        if (!IsMusicStreamPlaying(m)) PlayMusicStream(m);
    }
    static void Update() {
        for (auto &pair : musics) {
            if (IsMusicStreamPlaying(pair.second)) UpdateMusicStream(pair.second);
        }
    }
    static void UnloadAll(){
        for (auto &p: sounds) ::UnloadSound(p.second);
        for (auto &p: musics) ::UnloadMusicStream(p.second);
        sounds.clear(); musics.clear();
    }
private:
    static std::unordered_map<std::string, Sound> sounds;
    static std::unordered_map<std::string, Music> musics;
};
