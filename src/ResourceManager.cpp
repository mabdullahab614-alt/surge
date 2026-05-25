#include "ResourceManager.h"

// Define static member storage
std::unordered_map<std::string, Shader> ResourceManager::shaders;
std::unordered_map<std::string, Sound>  ResourceManager::sounds;
std::unordered_map<std::string, Music>  ResourceManager::musics;

// Optionally, you could add helper functions for unloading individual resources, but UnloadAll() is sufficient.
