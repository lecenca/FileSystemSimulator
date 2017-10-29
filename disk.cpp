#include "disk.h"

//#include <qdebug.h>

Disk::Disk()
{
    diskFile.open("disk.vdisk",
                  std::ios::in|std::ios::out|std::ios_base::binary);
}

Disk::~Disk(){
    diskFile.close();
}

Option<Block> Disk::readBlock(uint8_t index){
    if(index>=128)
        return Option<Block>();
    uint8_t block[64];
    diskFile.seekg(index*64,std::ios::beg);
    diskFile.read(reinterpret_cast<char*>(block),64);

    Block blk;
    for(unsigned i = 0;i<64;++i){
        blk[i] = block[i];
    }
    return Option<Block>(blk);
}

bool Disk::writeBlock(Block block, uint8_t index){
    if(index>=128)
        return false;
    diskFile.seekp(index*64,std::ios::beg);
    diskFile.write((char*)block.data(),64);
    diskFile.flush();
    return true;
}
