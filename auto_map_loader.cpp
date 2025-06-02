// auto_map_loader.cpp  
#include "auto_map_loader.h"  
#include "tileset.h"  
#include <sstream>  
  
std::set<uint32_t> collect_tileset_ids(const MapSegment& map_segment) {  
    std::set<uint32_t> ids;  
    for (uint32_t tile_info : map_segment.tiles) {  
        uint32_t tile_id = tile_info >> 8;  
        ids.insert(tile_id);  
    }  
    return ids;  
}  
  
std::set<uint32_t> collect_tileset_ids_from_segments(const std::vector<MapSegment>& segments) {  
    std::set<uint32_t> all_ids;  
    for (const auto& seg : segments) {  
        auto ids = collect_tileset_ids(seg);  
        all_ids.insert(ids.begin(), ids.end());  
          
        // 收集extra_floor_tiles的tile_id  
        for (const auto& [x, y, c] : seg.extra_floor_tiles) {  
            all_ids.insert(c >> 8);  
        }  
          
        // 收集objects的tile_id  
        for (const auto& obj : seg.objects) {  
            for (const auto& tile_data : obj.tiles) {  
                all_ids.insert(tile_data.data >> 8);  
            }  
        }  
    }  
    return all_ids;  
}  
  
std::unordered_map<uint32_t, std::vector<std::vector<uint16_t>>>   
load_needed_tilesets(Pack& pack, const std::set<uint32_t>& tileset_ids) {  
    std::unordered_map<uint32_t, std::vector<std::vector<uint16_t>>> tilesets;  
      
    for (uint32_t tile_id : tileset_ids) {  
        std::stringstream ss;  
        ss << tile_id << ".til";  
          
        auto tile_file_data = pack.raw_file_contents(ss.str());  
        if (!tile_file_data.empty()) {  
            auto tiles = decode_tileset_data(tile_file_data);  
            tilesets[tile_id] = std::move(tiles);  
        }  
    }  
      
    return tilesets;  
}