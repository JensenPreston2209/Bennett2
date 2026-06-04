#pragma once
#include <string>

struct SpawnPoint {
    float x = 0;
    float y = 0;
    std::string id;
    bool isDefault = false;
    bool isSafe = false;
};
