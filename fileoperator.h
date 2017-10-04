#ifndef FILEOPERATOR_H
#define FILEOPERATOR_H

#include "contentitem.h"
#include "disk.h"
#include "fileiter.h"
#include "option.h"
#include "openedlistitem.h"

#include <array>
#include <vector>
#include <stack>
#include <string>
#include <map>

using Name = std::array<uint8_t,3>;

/**
 * @brief FileOperator::FileOperator
 * 这个类叫文件操作器，主要负责文件的查找、新建、
 * 读取、删除、修改后保存等。
 */
class FileOperator
{
public:
    FileOperator();

    bool createFile(std::string path, uint8_t property);
    bool md(std::string path, uint8_t property);
    bool openFile(std::string path,uint8_t property);
    uint32_t readFile(std::string path,uint8_t* buff,uint32_t length); //返回真正读到的字节数
    bool writeFile(std::string path, uint8_t* buff, uint32_t length);
    bool closeFile(std::string path);
    bool deleteFile(std::string path);
    bool rd(std::string path);

public:
    static const uint8_t READMODEL = 0b00000000;
    static const uint8_t WRITEMODEL = 0b00000001;


private:
    uint8_t fat[128];
    Disk disk;
    std::map<std::string,OpenedListItem> openedList;

    std::vector<ContentItem> currentFolder;

private:
    ContentItem getRootItem();
    void openFolder(ContentItem item);
    Option<ContentItem> findContentItem(std::string path);
    Option<ContentItem> findFile(ContentItem folder, std::string& fileName);
    bool checkName(std::string path);
    std::tuple<uint8_t,uint8_t> findIndex(std::string path);
    Option<uint8_t> findEmptyBlock();
    bool createNewFile(std::string path, uint8_t property);
    bool deleteContent(ContentItem item);

    Option<uint8_t> getNext(FileIter& iter);
    std::vector<std::string> split(std::string str, char delimiter);
};

#endif // FILEOPERATOR_H
