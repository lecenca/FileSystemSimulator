#ifndef FILEITER_H
#define FILEITER_H

#include "contentitem.h"
#include "option.h"

#include <cinttypes>
#include <array>
#include <vector>
#include <stack>

using Block = std::array<uint8_t,64>;

/**
 * @brief The FileIter class
 * 拥有一个FileIter相当于拥有一个文件，
 * 准确地说，FileIter表示文件在内存中的
 * 一个副本。对文件的操作都通过对FileIter
 * 操作实现。注意，最后要进行写回磁盘，操作
 * 才会保存下来。
 */
class FileIter
{

public:
    FileIter();

    Option<uint8_t> next();
    Option<uint8_t> previous();
    void advance(uint8_t step);

public:
    uint8_t blockIndex;
    uint8_t innerIndex;
    uint8_t logicIndex;
    uint8_t length;


    std::stack<uint8_t> blkIndexStack;


};

#endif // FILEITER_H
