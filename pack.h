// pack.h  
#ifndef PACK_H  
#define PACK_H  
  
#include <string>  
#include <unordered_map>  
#include <vector>  
#include <cstdint>  
  
struct PackFileEntry {  
    std::string name;  
    uint32_t offset;  
    uint32_t size;  
      
    PackFileEntry(const std::string& name, uint32_t offset, uint32_t size)  
        : name(name), offset(offset), size(size) {}  
};  
  
class Pack {  
private:  
    std::string pak_path;  
    std::string idx_path;  
    std::unordered_map<std::string, PackFileEntry> entries;  
  
public:  
    Pack(const std::string& pak_path, const std::string& idx_path);  
    bool load();  
    std::vector<uint8_t> raw_file_contents(const std::string& name);  
};  
  
#endif  
  
