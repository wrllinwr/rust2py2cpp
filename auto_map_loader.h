// auto_map_loader.h  
#ifndef AUTO_MAP_LOADER_H  
#define AUTO_MAP_LOADER_H  
  
#include "types.h"  
#include "pack.h"  
#include <set>  
#include <unordered_map>  
  
std::set<uint32_t> collect_tileset_ids(const MapSegment& map_segment);  
std::set<uint32_t> collect_tileset_ids_from_segments(const std::vector<MapSegment>& segments);  
std::unordered_map<uint32_t, std::vector<std::vector<uint16_t>>>   
load_needed_tilesets(Pack& pack, const std::set<uint32_t>& tileset_ids);  
  
#endif  