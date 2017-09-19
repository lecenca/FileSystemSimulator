#ifndef STORAGER_H
#define STORAGER_H

#include "contentitem.h"
#include "disk.h"
#include "fileiter.h"

/**
 * @brief The Storager class
 * 这个类主要负责文件在磁盘上的创建、查找、删除等。
 * 或者说，磁盘空间的管理是它的主要任务。
 */
class Storager
{
public:
    Storager();

    ContentItem getRoot();
    FileIter getFile(ContentItem item);
//    FileIter getRoot();


private:
    uint8_t fat[128];
    Disk disk;

};

#endif // STORAGER_H
