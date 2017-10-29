#ifndef FILEITER_H
#define FILEITER_H

#include "contentitem.h"
#include "option.h"

#include <cinttypes>
#include <array>
#include <vector>
#include <stack>

using Block = std::array<uint8_t,64>;

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
