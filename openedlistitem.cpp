#include "openedlistitem.h"

OpenedListItem::OpenedListItem(std::string path, ContentItem item, uint8_t property):
    path(path),item(item),flag(property)
{
    fileIter.blockIndex = item.startPos;
    fileIter.innerIndex = 0;
    fileIter.logicIndex = 0;
    fileIter.length = item.length;
}
