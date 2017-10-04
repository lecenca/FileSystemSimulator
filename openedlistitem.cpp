#include "openedlistitem.h"

OpenedListItem::OpenedListItem(std::string path, ContentItem item, uint8_t openModel):
    path(path),item(item),openModel(openModel)
{
    fileIter.blockIndex = item.startPos;
    fileIter.innerIndex = 0;
    fileIter.logicIndex = 0;
    fileIter.length = item.length;
}
