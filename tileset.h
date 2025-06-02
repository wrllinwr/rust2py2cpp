// tileset.h  
#ifndef TILESET_H  
#define TILESET_H  
  
#include <vector>  
#include <cstdint>  
  
std::vector<std::vector<uint16_t>> decode_tileset_data(const std::vector<uint8_t>& data);  
  
#endif  