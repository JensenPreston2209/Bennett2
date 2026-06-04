#include "TileMap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>

#pragma warning(push)
#pragma warning(disable: 26819)
#include <json.hpp>
#pragma warning(pop)

using json = nlohmann::json;

TileMap::TileMap(SDL_Renderer* renderer)
    : renderer(renderer) {}

TileMap::~TileMap() {
    if (tileset) {
        SDL_DestroyTexture(tileset);
        tileset = nullptr;
    }
}

static std::string getDirectoryFromPath(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return "";
    return path.substr(0, pos + 1);
}

static std::string loadTSXImagePath(const std::string& tsxFullPath) {
    std::ifstream tsxFile(tsxFullPath);
    if (!tsxFile.is_open()) {
        std::cout << "Failed to open TSX file: " << tsxFullPath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << tsxFile.rdbuf();
    std::string content = buffer.str();

    std::size_t imagePos = content.find("<image");
    if (imagePos == std::string::npos) {
        std::cout << "No <image> tag found in TSX: " << tsxFullPath << std::endl;
        return "";
    }

    std::size_t srcPos = content.find("source=\"", imagePos);
    if (srcPos == std::string::npos) {
        std::cout << "No source=\"...\" in <image> tag in TSX: " << tsxFullPath << std::endl;
        return "";
    }

    srcPos += 8;
    std::size_t endPos = content.find("\"", srcPos);
    if (endPos == std::string::npos) {
        std::cout << "Unterminated source attribute in TSX: " << tsxFullPath << std::endl;
        return "";
    }

    return content.substr(srcPos, endPos - srcPos);
}

bool TileMap::loadJSON(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open map: " << path << std::endl;
        return false;
    }

    json j;
    file >> j;

    exits.clear();
    spawnPoints.clear();
    groundLayer.clear();
    collisionLayer.clear();

    if (j.contains("tilewidth"))  tileWidth = j["tilewidth"];
    if (j.contains("tileheight")) tileHeight = j["tileheight"];

    // -----------------------------
    // Load tileset (supports TSX)
    // -----------------------------
    if (j.contains("tilesets") && j["tilesets"].size() > 0) {
        std::string tilesetPath;
        std::string mapDir = getDirectoryFromPath(path);
        json ts = j["tilesets"][0];

        if (ts.contains("image")) {
            tilesetPath = ts["image"];
        }
        else if (ts.contains("source")) {
            std::string tsxRelPath = ts["source"];
            std::string tsxFullPath = mapDir + tsxRelPath;

            std::string imageRelPath = loadTSXImagePath(tsxFullPath);
            if (imageRelPath.empty()) {
                std::cout << "Failed to get image path from TSX: " << tsxFullPath << std::endl;
                return false;
            }

            tilesetPath = mapDir + imageRelPath;
        }
        else {
            std::cout << "Tileset missing image/source field." << std::endl;
            return false;
        }

        tileset = IMG_LoadTexture(renderer, tilesetPath.c_str());
        if (!tileset) {
            std::cout << "Failed to load tileset: " << tilesetPath
                << " | SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        float texW = 0.0f, texH = 0.0f;
        SDL_GetTextureSize(tileset, &texW, &texH);

        if (texW <= 0.0f) {
            std::cout << "ERROR: tileset width is 0." << std::endl;
            return false;
        }

        tilesetCols = static_cast<int>(texW) / tileWidth;
        if (tilesetCols <= 0) {
            std::cout << "ERROR: tilesetCols <= 0." << std::endl;
            return false;
        }

        std::cout << "Tileset loaded: " << tilesetPath
            << " | Width: " << texW
            << " | TileWidth: " << tileWidth
            << " | Columns: " << tilesetCols << std::endl;
    }

    // -----------------------------
    // Parse layers
    // -----------------------------
    if (j.contains("layers")) {
        for (auto& layer : j["layers"]) {
            std::string type = layer["type"];
            std::string name = layer["name"];

            // EXIT OBJECTS
            if (type == "objectgroup" && name == "exits") {
                for (auto& obj : layer["objects"]) {
                    ExitTrigger e;
                    e.x = (float)obj["x"];
                    e.y = (float)obj["y"];
                    e.w = (float)obj["width"];
                    e.h = (float)obj["height"];

                    if (obj.contains("properties")) {
                        for (auto& p : obj["properties"]) {
                            if (p["name"] == "exit_id")
                                e.exitID = p["value"];
                        }
                    }

                    exits.push_back(e);
                }
            }

            // SPAWN POINTS
            if (type == "objectgroup" && name == "spawns") {
                for (auto& obj : layer["objects"]) {
                    SpawnPoint sp;
                    sp.x = (float)obj["x"];
                    sp.y = (float)obj["y"];

                    if (obj.contains("properties")) {
                        for (auto& p : obj["properties"]) {
                            std::string pname = p["name"];
                            if (pname == "spawn_id") sp.id = p["value"];
                            else if (pname == "is_default") sp.isDefault = p["value"];
                            else if (pname == "is_safe") sp.isSafe = p["value"];
                        }
                    }

                    spawnPoints.push_back(sp);
                }
            }

            // GROUND LAYER
            if (type == "tilelayer" && name == "ground") {
                mapWidth = layer["width"];
                mapHeight = layer["height"];

                groundLayer.resize(mapHeight, std::vector<int>(mapWidth));

                int i = 0;
                for (int y = 0; y < mapHeight; y++)
                    for (int x = 0; x < mapWidth; x++)
                        groundLayer[y][x] = layer["data"][i++];
            }

            // COLLISION LAYER
            if (type == "tilelayer" && name == "collision") {
                collisionLayer.resize(mapHeight, std::vector<int>(mapWidth));

                int i = 0;
                for (int y = 0; y < mapHeight; y++)
                    for (int x = 0; x < mapWidth; x++)
                        collisionLayer[y][x] = layer["data"][i++];
            }
        }
    }

    return true;
}

void TileMap::render(int cameraX, int cameraY) {
    if (!tileset) return;

    SDL_FRect src;
    SDL_FRect dst;

    src.w = (float)tileWidth;
    src.h = (float)tileHeight;
    dst.w = (float)tileWidth;
    dst.h = (float)tileHeight;

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {

            int id = groundLayer[y][x];
            if (id <= 0) continue;

            id -= 1;

            src.x = (float)((id % tilesetCols) * tileWidth);
            src.y = (float)((id / tilesetCols) * tileHeight);

            dst.x = (float)(x * tileWidth - cameraX);
            dst.y = (float)(y * tileHeight - cameraY);

            SDL_RenderTexture(renderer, tileset, &src, &dst);
        }
    }
}

ExitTrigger* TileMap::checkExit(float px, float py, float pw, float ph) {
    for (auto& e : exits) {
        bool overlap =
            px + pw > e.x &&
            px < e.x + e.w &&
            py + ph > e.y &&
            py < e.y + e.h;

        if (overlap) return &e;
    }
    return nullptr;
}

SpawnPoint* TileMap::getSpawnByID(const std::string& id) {
    for (auto& sp : spawnPoints)
        if (sp.id == id)
            return &sp;
    return nullptr;
}

SpawnPoint* TileMap::getDefaultSpawn() {
    for (auto& sp : spawnPoints)
        if (sp.isDefault)
            return &sp;
    return nullptr;
}

bool TileMap::isBlocked(int tileX, int tileY) const {
    if (tileX < 0 || tileY < 0 || tileX >= mapWidth || tileY >= mapHeight)
        return true;

    return collisionLayer[tileY][tileX] != 0;
}

bool TileMap::collides(const SDL_FRect& box) const {
    int left = (int)(box.x / tileWidth);
    int right = (int)((box.x + box.w - 1) / tileWidth);
    int top = (int)(box.y / tileHeight);
    int bottom = (int)((box.y + box.h - 1) / tileHeight);

    return isBlocked(left, top) ||
        isBlocked(right, top) ||
        isBlocked(left, bottom) ||
        isBlocked(right, bottom);
}
