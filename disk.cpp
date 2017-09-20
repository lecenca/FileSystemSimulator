#include "disk.h"

#include <qdebug.h>

Disk::Disk()
{
    diskFile.open("disk.vdisk",
                  std::ios::in|std::ios::out|std::ios_base::binary);
}

Disk::~Disk(){
    diskFile.close();
}

Block Disk::readBlock(uint8_t index){
    //此处应检查index是否小于127
    //使用异常？静态断言？或其它？
    uint8_t block[64];
    diskFile.seekg(index*64,std::ios::beg);
    diskFile.read(reinterpret_cast<char*>(block),64);
    Block blk;
    for(unsigned i = 0;i<64;++i){
        blk[i] = block[i];
    }
    return blk;
}

void Disk::writeBlock(Block block, uint8_t index){
    diskFile.seekp(index*64,std::ios::beg);
    diskFile.write((char*)block.data(),64);
    diskFile.flush();
}
