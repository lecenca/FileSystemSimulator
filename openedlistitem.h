#ifndef OPENEDLISTITEM_H
#define OPENEDLISTITEM_H

#include "contentitem.h"
#include "fileiter.h"

#include <string>

class OpenedListItem
{
public:
    OpenedListItem(std::string path, ContentItem item, uint8_t openModel);

public:
    std::string path; //文件名
    ContentItem item;
    uint8_t openModel; //操作类型，0表示读，1表示写
    FileIter fileIter;
};

#endif // OPENEDLISTITEM_H
