#include "fileoperator.h"

#include <qdebug.h>

#include <iterator>
#include <tuple>

//文件打开都没关，记得关。

FileOperator::FileOperator()
{
    Block blk0 = disk.readBlock(0).some;
    Block blk1 = disk.readBlock(1).some;
    for(unsigned i = 0;i<64;++i){
        fat[i] = blk0[i];
        fat[64+i] = blk1[i];
    }
}

ContentItem FileOperator::getRootItem()
{
    ContentItem rootItem;
    Block blk = disk.readBlock(2).some;
    rootItem.name[0] = blk[0];
    rootItem.name[1] = blk[1];
    rootItem.name[2] = blk[2];
    rootItem.type[0] = blk[3];
    rootItem.type[1] = blk[4];
    rootItem.property = blk[5];
    rootItem.startPos = blk[6];
    rootItem.length = blk[7];
    return rootItem;
}



bool FileOperator::openFile(std::string path,uint8_t property)
{
    try{
        Option<ContentItem> result = findContentItem(path);

        if(result.none){
            return false;
        }
        /***/
        qInfo()<<"in FileOperator::openFile";
        qInfo()<<"here line 47";
        for(unsigned i = 0;i<8;++i)
            qInfo()<<result.some.toUint8Array()[i];
        /***/
        openedList.insert(
                    std::make_pair(
                        path,
                        OpenedListItem(path,result.some,property)));
        /***/
        qInfo()<<"in FileOperator::openFile";
        qInfo()<<"item.length ="<<result.some.length;
        /***/

        return true;
    }
    catch(std::exception e){
        qInfo()<<"break in FileOperator::openFile";
        throw e;
    }
}

uint32_t FileOperator::readFile(std::string path, uint8_t *buff, uint32_t length)
{
    try{
        OpenedListItem& openedItem = openedList.at(path);
        FileIter& iter = openedItem.fileIter;

        /***/
        qInfo()<<"in FileOperator::readFile";
        qInfo()<<"logicIndex: "<<iter.logicIndex;
        qInfo()<<"blockIndex: "<<iter.blockIndex;
        qInfo()<<"innerIndex: "<<iter.innerIndex;
        qInfo()<<"iter.length: "<<iter.length;
        /***/

        //指定的读出长度比文件剩余长度长，则只读剩余长度的的数据。
        if(iter.length-iter.logicIndex<length)
            length = iter.length - iter.logicIndex;
        uint8_t endIndex = iter.logicIndex + length;
        uint8_t buffIndex = 0;
        while(iter.logicIndex < endIndex){
            auto result = getNext(iter);
            if(result.none)
                break;
            buff[buffIndex] = result.some;
            ++buffIndex;
        }
        return length;
    }
    catch(std::out_of_range e){
        qInfo()<<"break in FileOperator::readFile";
        qInfo()<<"catch a out_of_range exception in FileOperator::readFile";
        throw e;
    }
    catch(std::exception e){
        qInfo()<<"break in FileOperator::readFile";
        throw e;
    }
}

//寻找folder文件夹里，名为fileName的文件
Option<ContentItem> FileOperator::findFile(ContentItem folder, std::string& fileName)
{
    if(folder.length==0)
        return Option<ContentItem>();

    bool isMenu = true;
    std::string typeName;
    QRegExp reg(QString("^[^\\$\\./]{3}\\.[^\\$\\./]{2}$"));
    if(reg.exactMatch(QString(fileName.data()))){
        typeName = fileName.substr(4,2);
        fileName = fileName.substr(0,3);
        isMenu = false;
    }

    Block block = disk.readBlock(folder.startPos).some;
    uint8_t blockIndex = folder.startPos;
    uint8_t innerIndex = 0;
    uint8_t logicIndex = 0;
    for(;logicIndex<folder.length;logicIndex+=8,innerIndex+=8){
        if(innerIndex>=64){
            innerIndex = innerIndex % 8;
            blockIndex = fat[blockIndex];
            block = disk.readBlock(blockIndex).some;
        }
        std::string name;
        name.append(1,(char)block.at(innerIndex));
        name.append(1,(char)block.at(innerIndex+1));
        name.append(1,(char)block.at(innerIndex+2));
        std::string type;
        type.append(1,(char)block.at(innerIndex+3));
        type.append(1,(char)block.at(innerIndex+4));
        if(fileName==name && (isMenu || (!isMenu && typeName==type))){
            auto iter = block.begin();
            std::advance(iter,innerIndex);
            ContentItem item = ContentItem::convertToItem(iter);
            return Option<ContentItem>(item);
        }
    }
    return Option<ContentItem>();
}

//未测试
std::vector<std::string> FileOperator::split(std::string str, char delimiter)
{
    std::vector<std::string> result;
    auto pos1 = 0;
    auto pos2 = str.find_first_of(delimiter);
    while(pos2 != std::string::npos){
        result.push_back(str.substr(pos1,pos2-pos1));
        pos1 = pos2 + 1;
        pos2 = str.find_first_of(delimiter,pos1);
    }
    result.push_back(str.substr(pos1,str.length()));
    return result;
}

Option<uint8_t> FileOperator::getNext(FileIter& iter)
{
    if(iter.logicIndex==iter.length)
        return Option<uint8_t>();
    Block block = disk.readBlock(iter.blockIndex).some;
    auto result =  Option<uint8_t>(block.at(iter.innerIndex));
    if(iter.innerIndex<63){
        ++iter.innerIndex;
    }else{
        iter.innerIndex = 0;
        iter.blockIndex = fat[iter.blockIndex];
    }
    ++iter.logicIndex;
    return result;
}

bool FileOperator::createNewFile(std::string path, uint8_t property)
{
    //此函数的大致逻辑如下
    /**
     * --检查各参数是否正确--不正确（创建失败）
     *                     \
     *                     \
     *                     \
     *                     正确
     *                     检查文件的父目录是否存在--不存在（创建失败）
     *                                             \
     *                                             \
     *                                             存在
     *                                             检查父目录是否已存在同名文件--存在（创建失败）
     *                                                                        \
     *                                                                        \
     *                                                                        不存在
     *                                                                        检查父目录内容所占最后一块盘块是否满--未满
     *                                                                                                          把新创建的ContentItem写入最后面
     *                                                                                                          修改父目录ContentItem的length并写回磁盘（创建成功）
     *                                                                                                          \
     *                                                                                                          \
     *                                                                                                          满
     *                                                                                                          查找空闲盘块--找不到（磁盘已满，创建失败）
     *                                                                                                                       \
     *                                                                                                                       \
     *                                                                                                                       找得到
     *                                                                                                                       把新创建的ContentItem写入找到的盘块
     *                                                                                                                       检查父目录原本是否为空-----------------------空
     *                                                                                                                                                                  修改父目录ContentItem的length为length+8
     *                                                                                                                                                                  把startPos改为刚刚找到的盘块的index
     *                                                                                                                                                                  把父目录ContentItem写回磁盘（创建成功）
     *                                                                                                                                                                  把fat中新盘块对应的项改为255
     *                                                                                                                                                                  把fat写回磁盘（创建成功）
     *                                                                                                                                                                  \
     *                                                                                                                                                                  \
     *                                                                                                                                                                  非空
     *                                                                                                                                                                  修改父目录ContentItem的length为length+8
     *                                                                                                                                                                  把父目录ContentItem写回磁盘
     *                                                                                                                                                                  把fat中父目录内容最后一块对应的项改为新盘块对应的项的index
     *                                                                                                                                                                  把fat中新盘块对应的项改为255
     *                                                                                                                                                                  把fat写回磁盘（创建成功）
     **/
    try{
        if(!checkName(path))
            return false;

        std::string type = "  ";
        std::string folderPath;
        std::string fileName;
        if(path.at(path.length()-3) == '.'){
            //新建的是文件
            folderPath = path.substr(0,path.length()-7);
            fileName = path.substr(path.length()-6,6);
            type = path.substr(path.length()-2,2);
        }else{
            //新建的是目录/文件夹
            folderPath = path.substr(0,path.length()-4);
            fileName = path.substr(path.length()-3,3);
        }

        //找到父目录的ContentItem
        Option<ContentItem> folderResult = findContentItem(folderPath);
        if(folderResult.none) //父目录不存在，新建失败
            return false;
        //查看文件是否已存在
        Option<ContentItem> fileResult = findFile(folderResult.some,fileName);
        if(!fileResult.none) //文件已存在，创建失败
            return false;
        /***/
        qInfo()<<"here";
        /***/

        //找到folder的内容，将新建item写入最后面
        ContentItem folder = folderResult.some;
        ContentItem item;
        item.setName(fileName.substr(0,3));
        item.setType(type);
        item.property = property;
        item.startPos = 255;
        item.length = 0;
        std::array<uint8_t,8> arr = item.toUint8Array();
        //如果该文件父目录所占的最后一个盘块没满，就将最后一块盘块找出来。
        if(folder.length % 64 != 0){
            uint8_t blockIndex = folder.startPos;
            uint8_t blockLength = folder.length / 64;
            for(int i = 0;i<blockLength;++i){
                blockIndex = fat[blockIndex];
            }
            uint8_t innerIndex = folder.length % 64;
            Block buff = disk.readBlock(blockIndex).some;
            //写入
            for(int i = 0;i<8;++i){
                buff[innerIndex+i] = arr[i];
            }
            disk.writeBlock(buff,blockIndex);
            //修改folder.length为folder.length+8，并写回磁盘
            std::tie(blockIndex,innerIndex) = findIndex(folderPath);
            buff = disk.readBlock(blockIndex).some;
            buff[innerIndex+7] += 8;
            disk.writeBlock(buff,blockIndex);
        }else{
            //如果该文件所占的最后一个盘块刚好满，或文件内容为空，则需要获取新的盘块，再将其写入
            auto result = findEmptyBlock();
            if(result.none)
                return false; //磁盘空间用完了
            uint8_t blockIndex = result.some;
            Block block;
            for(unsigned i = 0;i<8;++i){
                block[i] = arr[i];
            }
            disk.writeBlock(block,blockIndex);
            //修改fat
            //如果文件内容本来为空，则修改ContenItem的startPos,并将fat中blockIndex的位置置255，
            //使该ContentItem的length增加8，全部写回磁盘。
            if(folder.length==0){
                folder.startPos = blockIndex;
                fat[blockIndex] = 255;
                //将修改的ContentItem写回磁盘
                uint8_t blockIndex;
                uint8_t innerIndex;
                std::tie(blockIndex,innerIndex) = findIndex(folderPath);
                Block blk = disk.readBlock(blockIndex).some;
                std::array<uint8_t,8> array = folder.toUint8Array();
                for(unsigned i = 0;i<8;++i){
                    blk[innerIndex+i] = array[i];
                }
                disk.writeBlock(blk,blockIndex);
            }else{
                //如果文件内容不为空，则修改此文件内容对应的fat的
                //最后一项为blockIndex并将fat中blockIndex的位置置255
                uint8_t pos = folder.startPos;
                for(;;){
                    if(fat[pos]==255){
                        fat[pos] = blockIndex;
                        break;
                    }
                    pos = fat[pos];
                }
                fat[blockIndex] = (uint8_t)255;
            }
            //将修改的fat写回磁盘
            for(unsigned i = 0;i<64;++i)
                block[i] = fat[i];
            disk.writeBlock(block,0);
            for(unsigned i = 0;i<64;++i)
                block[i] = fat[64+i];
            disk.writeBlock(block,1);
        }
        return true;
    }
    catch(std::exception e){
        qInfo()<<"break in FileOperator::createNewFile";
        throw e;
    }
}

//找出路径对应的ContentItem对象
Option<ContentItem> FileOperator::findContentItem(std::string path)
{
    if(!checkName(path))
        return Option<ContentItem>();
    std::vector<std::string> pathVec = split(path,'/');
    ContentItem file = getRootItem();
    Option<ContentItem> result;
    pathVec.erase(pathVec.begin());
    for(auto str: pathVec){
        result = findFile(file, str);
        if(result.none)
            return Option<ContentItem>();
        file = result.some;
    }
    return Option<ContentItem>(file);
}

//未测试
//检查文件名是否合法
bool FileOperator::checkName(std::string path)
{
    QRegExp reg(QString("^rot(/[^\\$\\./]{3})*(/[^\\$\\./]{3}\\.[^\\$\\./]{2}){0,1}$"));
    return reg.exactMatch(QString(path.data()));
}

//找出ContentItem在磁盘中的位置，返回元组 (blockIndex,innerIndex)
std::tuple<uint8_t,uint8_t> FileOperator::findIndex(std::string path)
{
    //检查路径名是否正确
    if(!checkName(path))
        return std::make_tuple((uint8_t)255,(uint8_t)255); //这代表找不到
    //如果路径名为 rot。
    //则直接返回 rot 的ContentItem的位置。
    if(path==std::string("rot"))
        return std::make_tuple((uint8_t)2,(uint8_t)0);

    //如果路径名为 rot/.../bbb（也包括rot/bbb）
    //则先找出 bbb 的父目录的 ContentItem
    bool isMenu = true;
    std::string folderPath;
    std::string fileName;
    std::string typeName = "  ";
    if(path.at(path.length()-3)=='.'){
        //这是一个文件
        folderPath = path.substr(0,path.length()-7);
        fileName = path.substr(path.length()-6,3);
        typeName = path.substr(path.length()-2,2);
        isMenu = false;
    }else{
        //这是一个目录
        folderPath = path.substr(0,path.length()-4);
        fileName = path.substr(path.length()-3,3);
    }
    auto result = findContentItem(folderPath);
    /***/
    qInfo()<<"in FileOperator::findIndex";
    qInfo()<<"folderPath: "<<folderPath.data();
    qInfo()<<"fileName: "<<fileName.data();
    /***/
    if(result.none)
        return std::make_tuple((uint8_t)255,(uint8_t)255);
    ContentItem folder = result.some;
    //父目录的 contemItem 已找到， 现查找文件所在的位置。
    Block block = disk.readBlock(folder.startPos).some;
    uint8_t blockIndex = folder.startPos;
    uint8_t innerIndex = 0;
    uint8_t logicIndex = 0;
    for(;logicIndex<folder.length;logicIndex+=8,innerIndex+=8){
        if(innerIndex>=64){
            innerIndex = innerIndex % 8;
            blockIndex = fat[blockIndex];
            block = disk.readBlock(blockIndex).some;
        }

        std::string name, type;
        name.append(1,(char)block.at(innerIndex));
        name.append(1,(char)block.at(innerIndex+1));
        name.append(1,(char)block.at(innerIndex+2));
        type.append(1,(char)block.at(innerIndex+3));
        type.append(1,(char)block.at(innerIndex+4));
        if(fileName==name && (isMenu || (!isMenu && type==typeName))){
            return std::make_tuple((uint8_t)blockIndex,(uint8_t)innerIndex);
        }
    }
    return std::make_tuple((uint8_t)255,(uint8_t)255);
}

Option<uint8_t> FileOperator::findEmptyBlock()
{
    for(unsigned i = 0;i<128;++i){
        if(fat[i]==0)
            return Option<uint8_t>(i);
    }
    return Option<uint8_t>();
}

bool FileOperator::createFile(std::string path, uint8_t property)
{
    return createNewFile(path,property);
}

bool FileOperator::md(std::string path, uint8_t property)
{
    return createNewFile(path,property);
}

bool FileOperator::closeFile(std::string path)
{
    openedList.erase(path);
    return true;
}

bool FileOperator::deleteFile(std::string path)
{
    //此函数大致逻辑如下
    /**
     * 检查参数是否正确--不正确（删除失败）
     *                  \
     *                  \
     *                 正确
     *                 查找文件--文件不存在（删除失败）
     *                          \
     *                          \
     *                          文件存在
     *                          检查文件大小--文件大小为0--将此文件对应的ContentItem从父目录的内容中删去，
     *                                       \           将父目录的ContentItem的length-8，调整父目录的内容的分布。
     *                                       \           检查要删除的ContentItem是否是父目录内容的最后一个一个ContentItem-----------------------是
     *                                       \                                                                                             使父目录ContentItem的length减少8，并写回磁盘
     *                                       \                                                                                             检查此时父目录内容所占最后一块磁盘空间是否为空-----空
     *                                       \                                                                                             \                                             把fat中父目录内容所占最后一块磁盘空间对应的项置为0，倒数第二块对应的项置为255，并把fat写回磁盘。
     *                                       \                                                                                             \                                             \
     *                                       \                                                                                             \                                             \
     *                                       \                                                                                             \                                             不空
     *                                       \                                                                                             \                                             （删除成功）
     *                                       \                                                                                             \
     *                                       \                                                                                             \
     *                                       \                                                                                             不是
     *                                       \                                                                                             把最后一个ContentItem覆盖要删掉的此文件的ContentItem，把此块写回磁盘。
     *                                       \                                                                                             检查此时父目录内容所占最后一块磁盘空间是否为空-----------------------------空
     *                                       \                                                                                                                                                                   把fat中父目录内容所占最后一块磁盘空间对应的项置为0，倒数第二块对应的项置为255，并把fat写回磁盘。
     *                                       \                                                                                                                                                                   \
     *                                       \                                                                                                                                                                   \
     *                                       \                                                                                                                                                                   不空
     *                                       \                                                                                                                                                                   （删除成功）
     *                                       \
     *                                       \
     *                                       \
     *                                       \
     *                                       \
     *                                       \
     *                                       \
     *                                       文件大小不为0--修改fat，将分配给此文件的磁盘空间全部回收，并将fat写回硬盘
     *                                                     将此文件对应的ContentItem从父目录的内容中删去，
     *                                                     将父目录的ContentItem的length-8，调整父目录的内容的分布。
     *                                                     检查要删除的ContentItem是否是父目录内容的最后一个一个ContentItem-----------是
     *                                                                                                                           使父目录ContentItem的length减少8，并写回磁盘
     *                                                                                                                           检查此时父目录内容所占最后一块磁盘空间是否为空-----空
     *                                                                                                                           \                                             把fat中父目录内容所占最后一块磁盘空间对应的项置为255，并把fat写回磁盘。
     *                                                                                                                           \                                             \
     *                                                                                                                           \                                             \
     *                                                                                                                           \                                             不空
     *                                                                                                                           \                                             （删除成功）
     *                                                                                                                           \
     *                                                                                                                           \
     *                                                                                                                           不是
     *                                                                                                                           把最后一个ContentItem覆盖要删掉的此文件的ContentItem，把此块写回磁盘。
     *                                                                                                                           检查此时父目录内容所占最后一块磁盘空间是否为空-----------------------------空
     *                                                                                                                                                                                                 把fat中父目录内容所占最后一块磁盘空间对应的项置为255，并把fat写回磁盘。
     *                                                                                                                                                                                                 \
     *                                                                                                                                                                                                 \
     *                                                                                                                                                                                                 不空
     *                                                                                                                                                                                                 （删除成功）
     *
     */

    //检查参数是否正确
    QRegExp reg(QString("^rot(/[^\\$\\./]{3})*(/[^\\$\\./]{3}\\.[^\\$\\./]{2})$"));
    if(!reg.exactMatch(QString(path.data())))
        return false;

    if(path==std::string("rot")) //根目录不能删除，删除失败
        return false;
    Option<ContentItem> result = findContentItem(path);
    //检查文件是否存在
    if(result.none)
        return false; //不存在，删除失败
    ContentItem file = result.some;
    //获取父目录的ContentItem
    std::string folderName = path.substr(0,path.length()-7);
    ContentItem folder = findContentItem(folderName).some;
    //检查文件大小是否为0
    if(file.length!=0){
        //文件大小不为0,把分配给此文件的磁盘空间全部回收，最后将fat写回磁盘。
        uint8_t index = file.startPos;
        uint8_t t;
        while(index!=255){
            t = fat[index];
            fat[index] = 0;
            index = t;
        }
    }
    //调整父目录内容
    {
        //找到父目录内容的最后一块ContentItem的位置
        uint8_t lastBlockIndex;
        uint8_t t = folder.startPos;
        while(t!=255){
            lastBlockIndex = t;
            t = fat[t];
        }     
        uint8_t lastInnerIndex;
        lastInnerIndex = folder.length%64-8;
        //获取要删除的文件的ContentItem的位置
        uint8_t blockIndex, innerIndex;
        std::tie(blockIndex,innerIndex) = findIndex(path);
        /***/
        qInfo()<<"in FileOperator::deleteFile";
        qInfo()<<"lastBlockIndex: "<<lastBlockIndex;
        qInfo()<<"lastInnerIndex: "<<lastInnerIndex;
        qInfo()<<"blockIndex: "<<blockIndex;
        qInfo()<<"innerIndex: "<<innerIndex;
        /***/
        //如果要删除的ContentItem不是最后一块
        //用最后一块ContentItem覆盖要删除的ContentItem，并写回磁盘
        if(!(lastBlockIndex==blockIndex && lastInnerIndex==innerIndex)){
            Block buff1,buff2;
            buff1 = disk.readBlock(lastBlockIndex).some;
            buff2 = disk.readBlock(blockIndex).some;
            for(unsigned i = 0;i<8;++i){
                buff2[innerIndex+i] =  buff1[lastInnerIndex+i];
            }
            disk.writeBlock(buff2,blockIndex);
        }
        folder.length -= 8;
        //检查父目录内容中最后一块ContentItem被转移后，对应的磁盘空间
        //是否还有内容，若没有，在fat中将其标记为空闲，倒数第二块对应的值
        //置为255，最后将fat写回磁盘。
        if(folder.length % 64 == 0){
            //寻找父目录所占原倒数第二块磁盘空间的位置
            uint8_t secondLastBlockIndex;
            {
                uint8_t t = folder.startPos;
                while(t!=lastBlockIndex){
                    secondLastBlockIndex = t;
                    t = fat[t];
                }
            }
            fat[secondLastBlockIndex] = 255;
            fat[lastBlockIndex] = 0;
        }
    }

    //将父目录ContentItem的length-8并写回磁盘。
    {
        Block buff;
        uint8_t folderBlockIndex, folderInnerIndex;
        std::tie(folderBlockIndex,folderInnerIndex) = findIndex(folderName);
        buff = disk.readBlock(folderBlockIndex).some;
        /***/
        qInfo()<<"in FileOperator::deleteFile";
        qInfo()<<"folderBlockIndex: "<<folderBlockIndex;
        qInfo()<<"folderInnerIndex: "<<folderInnerIndex;
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+0];
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+1];
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+2];
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+3];
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+4];
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+5];
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+6];
        qInfo()<<"buff[folderInnerIndex+7]: "<<buff[folderInnerIndex+7];
        qInfo()<<"folder name: "<<QString(folderName.data());
        /***/
        buff[folderInnerIndex+7] -= 8;
        disk.writeBlock(buff,folderBlockIndex);
    }

    //把fat写回磁盘
    {
        Block buff;
        for(unsigned i = 0;i<64;++i)
            buff[i] = fat[i];
        disk.writeBlock(buff,0);
        for(unsigned i = 0;i<64;++i)
            buff[i] = fat[64+i];
        disk.writeBlock(buff,1);
    }
    return true;
}
