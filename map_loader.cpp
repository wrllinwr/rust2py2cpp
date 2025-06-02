#include "map_loader.h"  
#include <fstream>  
#include <sstream>  
#include <iomanip>  
#include <filesystem>  
  
std::string get_map_filename(const std::string& map_dir, int mapnum, int block_x, int block_y) {  
    int modx = (block_x >> 6) + 0x7e00;  
    int mody = (block_y >> 6) + 0x7e00;  
      
    std::stringstream ss;  
    ss << std::hex << std::setfill('0') << std::setw(4) << modx   
       << std::setw(4) << mody << ".s32";  
      
    return (std::filesystem::path(map_dir) / std::to_string(mapnum) / ss.str()).string();  
}  
  
MapSegment load_map_segment(const std::string& filename, int mapnum, int block_x, int block_y) {  
    std::ifstream file(filename, std::ios::binary);  
    if (!file) throw std::runtime_error("Cannot open file: " + filename);  
      
    // 读取瓦片数据  
    std::vector<uint32_t> tiles(MAP_W * 128);  
    file.read(reinterpret_cast<char*>(tiles.data()), tiles.size() * 4);  
      
    // 读取额外地面瓦片  
    uint16_t quant;  
    file.read(reinterpret_cast<char*>(&quant), 2);  
      
    std::vector<std::tuple<uint8_t, uint8_t, uint32_t>> extra_floor_tiles;  
    for (int i = 0; i < quant; ++i) {  
        uint8_t a, b;  
        uint32_t c;  
        file.read(reinterpret_cast<char*>(&a), 1);  
        file.read(reinterpret_cast<char*>(&b), 1);  
        file.read(reinterpret_cast<char*>(&c), 4);  
        extra_floor_tiles.emplace_back(a, b, c);  
    }  
      
    // 读取属性  
    std::vector<uint16_t> attributes(MAP_W * 128);  
    file.read(reinterpret_cast<char*>(attributes.data()), attributes.size() * 2);  
      
    // 读取对象  
    uint32_t object_count;  
    file.read(reinterpret_cast<char*>(&object_count), 4);  
      
    std::vector<MapObject> objects;  
    for (uint32_t i = 0; i < object_count; ++i) {  
        uint16_t index, num_tiles;  
        file.read(reinterpret_cast<char*>(&index), 2);  
        file.read(reinterpret_cast<char*>(&num_tiles), 2);  
          
        std::vector<TileData> tile_list;  
        for (int j = 0; j < num_tiles; ++j) {  
            uint8_t b, c, h;  
            file.read(reinterpret_cast<char*>(&b), 1);  
            file.read(reinterpret_cast<char*>(&c), 1);  
              
            if (b == 205 && c == 205) {  
                file.seekg(5, std::ios::cur);  
                continue;  
            }  
              
            file.read(reinterpret_cast<char*>(&h), 1);  
            uint32_t data;  
            file.read(reinterpret_cast<char*>(&data), 4);  
              
            tile_list.emplace_back(b, c, h, data);  
        }  
        objects.emplace_back(std::move(tile_list));  
    }  
      
    return MapSegment(mapnum, block_x, block_y, std::move(tiles),   
                     std::move(attributes), std::move(extra_floor_tiles),   
                     std::move(objects));  
}  
  
MapSegmentManager::MapSegmentManager(const std::string& map_dir, int mapnum, int view_w, int view_h)  
    : map_dir(map_dir), mapnum(mapnum), view_w(view_w), view_h(view_h) {}  
  
std::set<std::pair<int, int>> MapSegmentManager::needed_blocks(int center_x, int center_y) {  
    constexpr int tile_px = 24;  
    constexpr int tile_py = 12;  
    
    int buffer = 64;
    // int left = center_x - ((view_w / 2) / tile_px) * 2;  
    // int right = center_x + ((view_w / 2) / tile_px) * 2;  
    // int top = center_y - ((view_h / 2) / tile_py) * 2;  
    // int bottom = center_y + ((view_h / 2) / tile_py) * 2;  
    int left = center_x - ((view_w / 2) / tile_px) * 2 - buffer;  
    int right = center_x + ((view_w / 2) / tile_px) * 2 + buffer;  
    int top = center_y - ((view_h / 2) / tile_py) * 2 - buffer;  
    int bottom = center_y + ((view_h / 2) / tile_py) * 2 + buffer; 
      
    std::set<std::pair<int, int>> blocks;  
    for (int bx = (left & ~0x3F); bx <= (right & ~0x3F); bx += 64) {  
        for (int by = (top & ~0x3F); by <= (bottom & ~0x3F); by += 64) {  
            blocks.insert({bx, by});  
        }  
    }  
    return blocks;  
}  
  
void MapSegmentManager::update_segments(int center_x, int center_y) {  
    auto needed = needed_blocks(center_x, center_y);  
      
    // 加载新需要的  
    for (const auto& [bx, by] : needed) {  
        if (segments.find({bx, by}) == segments.end()) {  
            std::string fname = get_map_filename(map_dir, mapnum, bx, by);  
            if (std::filesystem::exists(fname)) {  
                segments.emplace(std::make_pair(bx, by),   
                               load_map_segment(fname, mapnum, bx, by));  
            }  
        }  
    }  
      
    // 卸载不再需要的  
    for (auto it = segments.begin(); it != segments.end();) {  
        if (needed.find(it->first) == needed.end()) {  
            it = segments.erase(it);  
        } else {  
            ++it;  
        }  
    }  
}  
  
std::vector<MapSegment> MapSegmentManager::get_active_segments() {  
    std::vector<MapSegment> result;  
    for (const auto& [key, segment] : segments) {  
        result.push_back(segment);  
    }  
    return result;  
}