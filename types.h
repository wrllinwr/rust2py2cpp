#ifndef TYPES_H  
#define TYPES_H  
  
#include <vector>  
#include <cstdint>  
  
constexpr int MAP_W = 64;  
constexpr int MAP_H = 64;  
  
struct TileData {  
    uint8_t x, y, h;  
    uint32_t data;  
      
    TileData(uint8_t x, uint8_t y, uint8_t h, uint32_t data)  
        : x(x), y(y), h(h), data(data) {}  
};  
  
struct MapObject {  
    std::vector<TileData> tiles;  
      
    MapObject(std::vector<TileData> tiles) : tiles(std::move(tiles)) {}  
};  
  
struct MapSegment {  
    int mapnum;  
    int x, y;  
    std::vector<uint32_t> tiles;  
    std::vector<uint16_t> attributes;  
    std::vector<std::tuple<uint8_t, uint8_t, uint32_t>> extra_floor_tiles;  
    std::vector<MapObject> objects;  
      
    MapSegment(int mapnum, int x, int y, std::vector<uint32_t> tiles,  
               std::vector<uint16_t> attributes,  
               std::vector<std::tuple<uint8_t, uint8_t, uint32_t>> extra_floor_tiles,  
               std::vector<MapObject> objects)  
        : mapnum(mapnum), x(x & ~0x3F), y(y & ~0x3F),   
          tiles(std::move(tiles)), attributes(std::move(attributes)),  
          extra_floor_tiles(std::move(extra_floor_tiles)), objects(std::move(objects)) {}  
};  
  
#endif