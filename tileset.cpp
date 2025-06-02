// tileset.cpp  
#include "tileset.h"  
#include <cstring>  
  
std::vector<std::vector<uint16_t>> decode_tileset_data(const std::vector<uint8_t>& data) {  
    if (data.size() < 4) return {};  
      
    const uint8_t* ptr = data.data();  
    uint16_t num_tiles = *reinterpret_cast<const uint16_t*>(ptr);  
    ptr += 4; // skip waste16  
      
    std::vector<uint32_t> offsets(num_tiles);  
    for (int i = 0; i < num_tiles; ++i) {  
        offsets[i] = *reinterpret_cast<const uint32_t*>(ptr);  
        ptr += 4;  
    }  
    ptr += 4; // skip w32  
      
    const uint8_t* base_offset = ptr;  
    std::vector<std::vector<uint16_t>> tiles;  
      
    for (uint32_t offset : offsets) {  
        const uint8_t* tile_ptr = base_offset + offset;  
        uint8_t v1 = *tile_ptr++;  
          
        std::vector<uint16_t> mirrored_tile_data(24 * 48, 0);  
          
        if ((v1 & 2) != 0) {  
            // 压缩镜像格式  
            uint8_t x = *tile_ptr++;  
            uint8_t y = *tile_ptr++;  
            tile_ptr++; // skip w  
            uint8_t h = *tile_ptr++;  
              
            for (int i = 0; i < h; ++i) {  
                uint8_t num_segments = *tile_ptr++;  
                int skip_index = 0;  
                  
                for (int j = 0; j < num_segments; ++j) {  
                    uint8_t num = *tile_ptr++;  
                    int skip = num / 2;  
                    uint8_t seg_width = *tile_ptr++;  
                    skip_index += skip;  
                      
                    for (int pixel = 0; pixel < seg_width; ++pixel) {  
                        uint16_t val = *reinterpret_cast<const uint16_t*>(tile_ptr);  
                        tile_ptr += 2;  
                          
                        int xx = x + skip_index + pixel;  
                        int yy = y + i;  
                        if (yy >= 0 && yy < 24 && xx >= 0 && xx < 48) {  
                            mirrored_tile_data[yy * 48 + xx] = val;  
                        }  
                    }  
                    skip_index += seg_width;  
                }  
            }  
        } else {  
            // 标准瓦片格式  
            std::vector<uint16_t> tile_data(288);  
            for (int i = 0; i < 288; ++i) {  
                tile_data[i] = *reinterpret_cast<const uint16_t*>(tile_ptr);  
                tile_ptr += 2;  
            }  
              
            int ind_offset = 0;  
            for (int i = 0; i < 24; ++i) {  
                int width = 2 * (i + 1);  
                if (i > 11) width -= 4 * (i - 11);  
                  
                int left_start = 24 - width;  
                int right_start = 24;  
                  
                for (int j = 0; j < width; ++j) {  
                    uint16_t d = tile_data[ind_offset++];  
                    mirrored_tile_data[i * 48 + left_start + j] = d;  
                    mirrored_tile_data[i * 48 + right_start + j] = d;  
                }  
            }  
        }  
          
        tiles.push_back(std::move(mirrored_tile_data));  
    }  
      
    return tiles;  
}