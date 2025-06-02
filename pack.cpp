// pack.cpp  
#include "pack.h"  
#include <fstream>  
#include <algorithm>  
#include <cctype>  
#include <cstring>
  
Pack::Pack(const std::string& pak_path, const std::string& idx_path)  
    : pak_path(pak_path), idx_path(idx_path) {}  
  
bool Pack::load() {  
    std::ifstream file(idx_path, std::ios::binary);  
    if (!file) return false;  
      
    uint32_t count;  
    file.read(reinterpret_cast<char*>(&count), 4);  
      
    for (uint32_t i = 0; i < count; ++i) {  
        uint32_t offset;  
        file.read(reinterpret_cast<char*>(&offset), 4);  
          
        char name_buf[20];  
        file.read(name_buf, 20);  
        std::string name(name_buf, strnlen(name_buf, 20));  
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);  
          
        uint32_t size;  
        file.read(reinterpret_cast<char*>(&size), 4);  
          
        entries.emplace(name, PackFileEntry(name, offset, size));  
    }  
      
    return true;  
}  
  
std::vector<uint8_t> Pack::raw_file_contents(const std::string& name) {  
    std::string lower_name = name;  
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);  
      
    auto it = entries.find(lower_name);  
    if (it == entries.end()) return {};  
      
    std::ifstream file(pak_path, std::ios::binary);  
    if (!file) return {};  
      
    file.seekg(it->second.offset);  
    std::vector<uint8_t> data(it->second.size);  
    file.read(reinterpret_cast<char*>(data.data()), it->second.size);  
      
    return data;  
}