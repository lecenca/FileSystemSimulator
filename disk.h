#ifndef DISK_H
#define DISK_H

#include "option.h"

#include <fstream>
#include <array>

using Block = std::array<uint8_t,64>;

class Disk
{
public:
    Disk();
    ~Disk();

    //由于读磁盘只能一块一块读，并且设定一个盘块64B，
    //所以用这个函数假装一次只能读64B。
    Option<Block> readBlock(uint8_t index);
    bool writeBlock(Block block, uint8_t index);

private:
    std::fstream diskFile;
};

#endif // DISK_H
