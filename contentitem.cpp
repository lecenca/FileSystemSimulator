#include "contentitem.h"

std::array<uint8_t, 8> ContentItem::toUint8Array(){
    std::array<uint8_t,8> arr;
    arr[0] = name[0];
    arr[1] = name[1];
    arr[2] = name[2];
    arr[3] = type[0];
    arr[4] = type[1];
    arr[5] = property;
    arr[6] = startPos;
    arr[7] = length;
    return arr;
}

void ContentItem::setName(std::string name){
    this->name[0] = name[0];
    this->name[1] = name[1];
    this->name[2] = name[2];
}

void ContentItem::setType(std::string type){
    this->type[0] = type[0];
    this->type[1] = type[1];
}
