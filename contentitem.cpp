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

std::string ContentItem::getFileName()
{
    std::string fileName;
    fileName.push_back((char)name[0]);
    fileName.push_back((char)name[1]);
    fileName.push_back((char)name[2]);
    if((property&ContentItem::MENU)!=ContentItem::MENU){
        //是个文件
       fileName.push_back('/');
       fileName.push_back((char)type[0]);
       fileName.push_back((char)type[1]);
    }
    return fileName;
}
