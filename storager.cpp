#include "storager.h"

#include <QtDebug>

Storager::Storager()
{
    //改
    Block blk0 = disk.readBlock(0);
    Block blk1 = disk.readBlock(1);

    for(unsigned i = 0;i<64;++i){
        fat[i] = blk0[i];
        fat[64+i] = blk1[i];
    }
}

ContentItem Storager::getRoot(){
    Block blk = disk.readBlock(2);
    ContentItem rootItem;
    rootItem.name[0] = blk[0];
    rootItem.name[1] = blk[1];
    rootItem.name[2] = blk[2];
    rootItem.type[0] = blk[3];
    rootItem.type[1] = blk[4];
    rootItem.property = blk[5];
    rootItem.startPos = blk[6];
    rootItem.length = blk[7];
    return rootItem;
}

FileIter Storager::getFile(ContentItem item){
    //从item找出起始盘块
    //根据fat与从item得出的文件长度读入相应盘块
    //构造FileIter并返回
    FileIter file{};
}
