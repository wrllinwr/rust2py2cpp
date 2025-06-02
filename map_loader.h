// map_loader.h  
#ifndef MAP_LOADER_H  
#define MAP_LOADER_H  
  
#include "types.h"  
#include <string>  
#include <unordered_map>  
#include <set>  
#include <functional>  
  
// 为std::pair<int,int>提供哈希函数  
struct PairHash {  
    std::size_t operator()(const std::pair<int, int>& p) const {  
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);  
    }  
};  
  
std::string get_map_filename(const std::string& map_dir, int mapnum, int block_x, int block_y);  
MapSegment load_map_segment(const std::string& filename, int mapnum, int block_x, int block_y);  
  
class MapSegmentManager {  
private:  
    std::string map_dir;  
    int mapnum;  
    int view_w, view_h;  
    // 使用自定义哈希函数  
    std::unordered_map<std::pair<int, int>, MapSegment, PairHash> segments;  
  
public:  
    MapSegmentManager(const std::string& map_dir, int mapnum, int view_w = 1200, int view_h = 900);  
    std::set<std::pair<int, int>> needed_blocks(int center_x, int center_y);  
    void update_segments(int center_x, int center_y);  
    std::vector<MapSegment> get_active_segments();  
};  
  
#endif