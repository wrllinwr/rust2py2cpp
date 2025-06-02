#include <SDL2/SDL.h>  
#include <iostream>  
#include <unordered_map>  
#include <algorithm>  
#include "pack.h"  
#include "map_loader.h"  
#include "auto_map_loader.h"  
#include "types.h"  
  
struct RenderTile {  
    std::string type;  
    int h;  
    int world_x, world_y;  
    uint32_t tile_id;  
    uint8_t subtile;  
    bool half;  
};  
  
SDL_Texture* rgb555_to_texture(SDL_Renderer* renderer, const std::vector<uint16_t>& tile_data) {  
    std::vector<uint32_t> rgba_data(48 * 24);  
      
    for (int i = 0; i < 24 * 48; ++i) {  
        uint16_t pixel = tile_data[i];  
        uint8_t r = ((pixel >> 10) & 0x1f) << 3;  
        uint8_t g = ((pixel >> 5) & 0x1f) << 3;  
        uint8_t b = (pixel & 0x1f) << 3;  
        uint8_t a = (pixel == 0) ? 0 : 255; // 透明度处理  
          
        rgba_data[i] = (a << 24) | (r << 16) | (g << 8) | b;  
    }  
      
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(  
        rgba_data.data(), 48, 24, 32, 48 * 4,  
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);  
      
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));  
      
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);  
    SDL_FreeSurface(surface);  
      
    return texture;  
}  

/** 这样会把所有的黑色的像素都变成透明，很显然阴影不能这么处理
SDL_Texture* rgb555_to_texture(SDL_Renderer* renderer, const std::vector<uint16_t>& tile_data) {  
    std::vector<uint32_t> rgba_data(48 * 24);  
      
    for (int i = 0; i < 24 * 48; ++i) {  
        uint16_t pixel = tile_data[i];  
        uint8_t r = ((pixel >> 10) & 0x1f) << 3;  
        uint8_t g = ((pixel >> 5) & 0x1f) << 3;  
        uint8_t b = (pixel & 0x1f) << 3;  
          
        // 改进透明度处理 - 支持阴影的半透明效果  
        uint8_t a = 255;  
        if (pixel == 0) {  
            a = 0; // 完全透明  
        } else if (r < 32 && g < 32 && b < 32) {  
            // 检测暗色像素作为阴影，设置半透明  
            a = 128;  
        }  
          
        rgba_data[i] = (a << 24) | (r << 16) | (g << 8) | b;  
    }  
      
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(  
        rgba_data.data(), 48, 24, 32, 48 * 4,  
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);  
      
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);  
    SDL_FreeSurface(surface);  
      
    // 设置混合模式支持透明度  
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);  
      
    return texture;  
} */

int main() {  
    const std::string map_dir = "/home/wrl/Lineage/map";  
    const int mapnum = 4;  
    int x = 33068, y = 32806;  
    int offset_x = 400, offset_y = 200;  
      
    MapSegmentManager manager(map_dir, mapnum);  
    int center_x = x, center_y = y;  
      
    Pack pack("/home/wrl/Lineage/Tile.pak", "/home/wrl/Lineage/Tile.idx");  
    if (!pack.load()) {  
        std::cerr << "Failed to load pack files" << std::endl;  
        return -1;  
    }  
      
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {  
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;  
        return -1;  
    }  
      
    SDL_Window* window = SDL_CreateWindow("Map Render",   
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,   
        800, 600, SDL_WINDOW_SHOWN);  
      
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);  
      
    std::unordered_map<uint32_t, std::vector<SDL_Texture*>> tile_textures;  
    std::set<uint32_t> loaded_tilesets;  
      
    bool running = true;  
    const int MOVE_X = 24, MOVE_Y = 12;  
      
    while (running) {  
        SDL_Event event;  
        while (SDL_PollEvent(&event)) {  
            if (event.type == SDL_QUIT) {  
                running = false;  
            } else if (event.type == SDL_KEYDOWN) {  
                switch (event.key.keysym.sym) {  
                    case SDLK_LEFT: case SDLK_a:  
                        offset_x += MOVE_X; center_x -= 2; break;  
                    case SDLK_RIGHT: case SDLK_d:  
                        offset_x -= MOVE_X; center_x += 2; break;  
                    case SDLK_UP: case SDLK_w:  
                        offset_y += MOVE_Y; center_y -= 2; break;  
                    case SDLK_DOWN: case SDLK_s:  
                        offset_y -= MOVE_Y; center_y += 2; break;  
                }  
            }  
        }  
          
        // 动态加载/卸载分块  
        manager.update_segments(center_x, center_y);  
        auto segments = manager.get_active_segments();  
          
        // 动态收集所有用到的tile_id，并准备贴图  
        auto needed_tileset_ids = collect_tileset_ids_from_segments(segments);  
        std::set<uint32_t> new_needed;  
        std::set_difference(needed_tileset_ids.begin(), needed_tileset_ids.end(),  
                           loaded_tilesets.begin(), loaded_tilesets.end(),  
                           std::inserter(new_needed, new_needed.begin()));  
          
        if (!new_needed.empty()) {  
            auto tilesets = load_needed_tilesets(pack, new_needed);  
            for (const auto& [tile_id, tile_list] : tilesets) {  
                std::vector<SDL_Texture*> textures;  
                for (const auto& tile_data : tile_list) {  
                    textures.push_back(rgb555_to_texture(renderer, tile_data));  
                }  
                tile_textures[tile_id] = std::move(textures);  
            }  
            loaded_tilesets.insert(new_needed.begin(), new_needed.end());  
        }  
          
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  
        SDL_RenderClear(renderer);  
          
        // 收集所有渲染物体  
        std::vector<RenderTile> all_render_tiles;  
          
        for (const auto& seg : segments) {  
            // extra_floor_tiles  
            for (const auto& [x, y, c] : seg.extra_floor_tiles) {  
                all_render_tiles.push_back({  
                    "extra", 0, seg.x + (x / 2), seg.y + y,  
                    c >> 8, static_cast<uint8_t>(c & 0xFF), static_cast<bool>(x % 2)  
                });  
            }  
              
            // objects  
            for (const auto& obj : seg.objects) {  
                for (const auto& tdata : obj.tiles) {  
                    all_render_tiles.push_back({  
                        "object", tdata.h, seg.x + (tdata.x / 2), seg.y + tdata.y,  
                        tdata.data >> 8, static_cast<uint8_t>(tdata.data & 0xFF),   
                        static_cast<bool>(tdata.x % 2)  
                    });  
                }  
            }  
        }  
          
        // 排序  去掉排序也没有阴影。验证了阴影被覆盖的观点，证明rust版本就是没有获得阴影。
        std::sort(all_render_tiles.begin(), all_render_tiles.end(),  
                 [](const RenderTile& a, const RenderTile& b) {  
                     if (a.h != b.h) return a.h < b.h;  
                     if (a.world_y != b.world_y) return a.world_y < b.world_y;  
                     return a.world_x < b.world_x;  
                 });  
          
        // 渲染地板  
        for (const auto& seg : segments) {  
            for (int a = 0; a < MAP_W; ++a) {  
                for (int b = 0; b < MAP_H; ++b) {  
                    int idx_left = b * 128 + 2 * a;  
                    int idx_right = b * 128 + 2 * a + 1;  
                    int world_x = seg.x + a;  
                    int world_y = seg.y + b;  
                      
                    auto render_tile = [&](int idx, int x_offset) {  
                        if (idx < static_cast<int>(seg.tiles.size())) {  
                            uint32_t tile_info = seg.tiles[idx];  
                            uint32_t tile_id = tile_info >> 8;  
                            uint8_t subtile = tile_info & 0xFF;  
                              
                            auto it = tile_textures.find(tile_id);  
                            if (it != tile_textures.end() && subtile < it->second.size()) {  
                                SDL_Texture* tex = it->second[subtile];  
                                int px = (world_x - center_x) * 24 + (world_y - center_y) * 24 + offset_x + x_offset;  
                                int py = (world_y - center_y) * 12 - (world_x - center_x) * 12 + offset_y;  
                                SDL_Rect src_rect = {x_offset, 0, 24, 24};  
                                SDL_Rect dst_rect = {px, py, 24, 24};  
                                SDL_RenderCopy(renderer, tex, &src_rect, &dst_rect);  
                            }  
                        }  
                    };  
                      
                    // 左半格  
                    render_tile(idx_left, 0);  
                    // 右半格    
                    render_tile(idx_right, 24);  
                }  
            }  
        }  
          
        // 按排序后的全局渲染列表画出所有建筑和地面物体  
        for (const auto& t : all_render_tiles) {  
            int px = (t.world_x - center_x) * 24 + (t.world_y - center_y) * 24 + offset_x;  
            if (t.half) px += 24;  
            int py = (t.world_y - center_y) * 12 - (t.world_x - center_x) * 12 + offset_y;  
              
            auto it = tile_textures.find(t.tile_id);  
            if (it != tile_textures.end() && t.subtile < it->second.size()) {  
                SDL_Texture* tex = it->second[t.subtile];  
                SDL_Rect dst_rect = {px, py, 48, 24};  
                SDL_RenderCopy(renderer, tex, nullptr, &dst_rect);  
            }  
        }  
          
        SDL_RenderPresent(renderer);  
        SDL_Delay(16);  
    }  
      
    // 清理  
    for (auto& [tile_id, textures] : tile_textures) {  
        for (SDL_Texture* tex : textures) {  
            if (tex) SDL_DestroyTexture(tex);  
        }  
    }  
      
    SDL_DestroyRenderer(renderer);  
    SDL_DestroyWindow(window);  
    SDL_Quit();  
      
    return 0;  
}