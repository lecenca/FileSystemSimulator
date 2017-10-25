#ifndef CONTENTITEM_H
#define CONTENTITEM_H

#include <cstdint>
#include <array>

//目录项、文件项一律用ContentItem表示
class ContentItem{
public:
    uint8_t name[3];  //目录、文件名字
    uint8_t type[2];  //文件类型，概念上相当于jpg、png、txt等，对于目录来说是没有这个属性的。
    uint8_t property; //属性，一些对文件、目录来说通用的系统属性，如只读、可读写等。
    uint8_t startPos; //起始位置
    uint8_t length;   //实验要求以盘块为单位，但这里选择以字节为单位。

    static const uint8_t MENU = 0b00001000;
    static const uint8_t NORMAL = 0b00000100;
    static const uint8_t SYSTEM = 0b00000010;
    static const uint8_t READONLY = 0b00000001;

    template<class Iter>
    static ContentItem convertToItem(Iter iter){
        ContentItem item;
        item.name[0] = *iter; ++iter;
        item.name[1] = *iter; ++iter;
        item.name[2] = *iter; ++iter;
        item.type[0] = *iter; ++iter;
        item.type[1] = *iter; ++iter;
        item.property = *iter; ++iter;
        item.startPos = *iter; ++iter;
        item.length = *iter;
        return item;
    }

    void setName(std::string name);
    void setType(std::string type);
    std::array<uint8_t,8> toUint8Array();
    std::string getFileName();

};

#endif // CONTENTITEM_H
