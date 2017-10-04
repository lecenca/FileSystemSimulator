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
        //检查文件是否打开
        if(openedList.find(path)==openedList.end())
            return false;

        OpenedListItem& openedItem = openedList.at(path);
        //检查是否以读模式打开
        if(openedItem.openModel==FileOperator::WRITEMODEL)
            return false;
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

    /***/
    qInfo()<<"in FileOperator::findFile";
    qInfo()<<"(before)fileName: "<<fileName.data();
    qInfo()<<"(before)isMenu: "<<isMenu;
    /***/

    if(fileName.length()==6){
        typeName = fileName.substr(4,2);
        fileName = fileName.substr(0,3);
        isMenu = false;
    }
    /***/
    qInfo()<<"in FileOperator::findFile";
    qInfo()<<"(after)fileName: "<<fileName.data();
    qInfo()<<"(after)isMenu: "<<isMenu;
    /***/

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
        uint8_t property = block.at(innerIndex+5);
        if(fileName==name &&
                (isMenu &&  property == ContentItem::MENU ||
                 (!isMenu && (property == ContentItem::NORMAL) && typeName==type))){
            auto iter = block.begin();
            std::advance(iter,innerIndex);
            ContentItem item = ContentItem::convertToItem(iter);
            return Option<ContentItem>(item);
        }
    }
    /***/
    qInfo()<<"in FileOperator::findFile";
    qInfo()<<"here 165";
    /***/
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

        //找到folder的内容，将新建item写入最后面
        ContentItem folder = folderResult.some;
        ContentItem item;
        item.setName(fileName.substr(0,3));
        item.setType(type);
        item.property = property;
        item.startPos = 255;
        item.length = 0;
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
            std::array<uint8_t,8> arr = item.toUint8Array();
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
            uint8_t emptyBlockIndex = result.some;
            Block block;
            std::array<uint8_t,8> arr = item.toUint8Array();
            for(unsigned i = 0;i<8;++i){
                block[i] = arr[i];
            }
            disk.writeBlock(block,emptyBlockIndex);
            //修改fat
            //如果文件内容本来为空，则修改ContenItem的startPos,并将fat中blockIndex的位置置255，
            //使该ContentItem的length增加8，全部写回磁盘。
            ContentItem modifiedFolder = folder;
            if(folder.length==0){
                modifiedFolder.startPos = emptyBlockIndex;
                fat[emptyBlockIndex] = 255;
                modifiedFolder.length += 8;
            }else{
                //如果文件内容不为空，则修改此文件内容对应的fat的
                //最后一项为blockIndex并将fat中blockIndex的位置置255
                uint8_t pos = folder.startPos;
                for(;;){
                    if(fat[pos]==255){
                        fat[pos] = emptyBlockIndex;
                        break;
                    }
                    pos = fat[pos];
                }
                fat[emptyBlockIndex] = 255;
                modifiedFolder.length += 8;
            }
            //将修改的fat写回磁盘
            for(unsigned i = 0;i<64;++i)
                block[i] = fat[i];
            disk.writeBlock(block,0);
            for(unsigned i = 0;i<64;++i)
                block[i] = fat[64+i];
            disk.writeBlock(block,1);
            //将修改的父目录的contentItem写回磁盘
            {
                uint8_t blockIndex;
                uint8_t innerIndex;
                std::tie(blockIndex,innerIndex) = findIndex(folderPath);
                Block blk = disk.readBlock(blockIndex).some;
                std::array<uint8_t,8> array = modifiedFolder.toUint8Array();
                for(unsigned i = 0;i<8;++i){
                    blk[innerIndex+i] = array[i];
                }
                disk.writeBlock(blk,blockIndex);
            }
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
        uint8_t property;
        name.append(1,(char)block.at(innerIndex));
        name.append(1,(char)block.at(innerIndex+1));
        name.append(1,(char)block.at(innerIndex+2));
        type.append(1,(char)block.at(innerIndex+3));
        type.append(1,(char)block.at(innerIndex+4));
        property = block.at(innerIndex+5);
        if(fileName==name &&
                (isMenu &&  property == ContentItem::MENU ||
                 (!isMenu && (property == ContentItem::NORMAL) && typeName==type))){
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
     *                                                                                                                           检查此时父目录内容所占最后一块磁盘空间是否为空-----空
     *                                                                                                                           \                                             把fat中父目录内容所占最后一块磁盘空间对应的项置为0，倒数第二块置为255，并把fat写回磁盘。
     *                                                                                                                           \                                             \
     *                                                                                                                           \                                             \
     *                                                                                                                           \                                             不空
     *                                                                                                                           \                                             （删除成功）
     *                                                                                                                           \
     *                                                                                                                           \
     *                                                                                                                           不是
     *                                                                                                                           把最后一个ContentItem覆盖要删掉的此文件的ContentItem，把此块写回磁盘。
     *                                                                                                                           检查此时父目录内容所占最后一块磁盘空间是否为空-----------------------------空
     *                                                                                                                                                                                                 把fat中父目录内容所占最后一块磁盘空间对应的项置为0，倒数第二块置为255，并把fat写回磁盘。
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
    //检查是否被打开
    if(openedList.find(path)!=openedList.end())
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

bool FileOperator::rd(std::string path)
{
    /**
      *  检查参数是否正确--不正确（删除失败）
      *                  \
      *                  \
      *                  正确
      *                  检查目录是否存在--不存在（删除失败）
      *                                  \
      *                                  \
      *                                  存在
      *                                  使用递归删除整个目录
      *                                  将此目录对应的ContentItem从父目录的内容中删去
      *                                  将父目录的ContentItem的length-8，调整父目录的内容的分布
      *                                  检查要删除的ContentItem是否是父目录内容的最后一个一个ContentItem----是
      *                                                                                                 检查此时父目录内容所占最后一块磁盘空间是否为空--空
      *                                                                                                 \                                          把fat中父目录内容所占最后一块磁盘空间对应的项置为0，倒数第二块置为255，并把fat写回磁盘。
      *                                                                                                 \                                          \
      *                                                                                                 \                                          \
      *                                                                                                 \                                          不空（删除成功）
      *                                                                                                 \
      *                                                                                                 \
      *                                                                                                 不是
      *                                                                                                 把最后一个ContentItem覆盖要删掉的此文件的ContentItem，把此块写回磁盘。
      *                                                                                                 检查此时父目录内容所占最后一块磁盘空间是否为空-----------------------------空
      *                                                                                                                                                                       把fat中父目录内容所占最后一块磁盘空间对应的项置为0，倒数第二块置为255，并把fat写回磁盘。
      *                                                                                                                                                                       \
      *                                                                                                                                                                       \
      *                                                                                                                                                                       不空（删除成功）
      *
      */

    //检查属性，如果不能删，则删除失败。（待完成）（大改）

    //检查参数是否正确
    QRegExp reg(QString("^rot(/[^\\$\\./]{3})*$"));
    if(!reg.exactMatch(QString(path.data())))
        return false;
    //检查目录是否存在
    Option<ContentItem> result = findContentItem(path);
    if(result.none)
        return false;
    //检查是否被打开
    if(openedList.find(path)!=openedList.end())
        return false;
    ContentItem folder = result.some;
    //删除folder的内容
    deleteContent(folder);
    //获取父目录（父文件夹）的ContentItem
    std::string fatherFolderName = path.substr(0,path.length()-4);
    ContentItem fatherFolder = findContentItem(fatherFolderName).some;
    //调整父目录内容
    {
        //找到父目录内容的最后一块ContentItem的位置
        uint8_t lastBlockIndex;
        uint8_t t = fatherFolder.startPos;
        while(t!=255){
            lastBlockIndex = t;
            t = fat[t];
        }
        uint8_t lastInnerIndex;
        lastInnerIndex = fatherFolder.length%64-8;
        //获取要删除的文件的ContentItem的位置
        uint8_t blockIndex, innerIndex;
        std::tie(blockIndex,innerIndex) = findIndex(path);
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
        fatherFolder.length -= 8;
        //检查父目录内容中最后一块ContentItem被转移后，对应的磁盘空间
        //是否还有内容，若没有，在fat中将其标记为空闲，倒数第二块对应的值
        //置为255，最后将fat写回磁盘。
        if(fatherFolder.length % 64 == 0){
            //寻找父目录所占原倒数第二块磁盘空间的位置
            uint8_t secondLastBlockIndex;
            {
                uint8_t t = fatherFolder.startPos;
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
        std::tie(folderBlockIndex,folderInnerIndex) = findIndex(fatherFolderName);
        buff = disk.readBlock(folderBlockIndex).some;
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

//递归地回收给定ContentItem的内容所占的全部磁盘空间，但不删除这个ContentItem本身
bool FileOperator::deleteContent(ContentItem item)
{
    //检查属性，如果不能删，则删除失败。(待完成)

    //如果这个contentItem是普通文件的contentItem
    if(item.property == ContentItem::NORMAL){
        if(item.length==0)
            return true;
        //回收此contentItem的内容所占的磁盘空间
        uint8_t blockIndex = item.startPos;
        uint8_t next;
        while(blockIndex != 255){
            next = fat[blockIndex];
            fat[blockIndex] = 0;
            blockIndex = next;
        }
    }else{
        //如果这个contentItem是文件夹的contentItem
        if(item.length==0)
            return true;
         //读入这个文件夹的内容，并一一回收里面每个文件和文件夹所分配的磁盘空间。
        {
            uint8_t logicIndex = 0;
            uint8_t blockIndex = item.startPos;
            uint8_t innerIndex = 0;
            std::vector<ContentItem> contentItemList;
            Block buff = disk.readBlock(blockIndex).some;
            while(logicIndex<item.length){
                auto iter = buff.cbegin();
                std::advance(iter,innerIndex);
                contentItemList.push_back(ContentItem::convertToItem(iter));
                logicIndex += 8;
                innerIndex += 8;
                if(innerIndex >= 64){
                    blockIndex = fat[blockIndex];
                    innerIndex %= 64;
                }
            }
            //删除
            for(ContentItem item: contentItemList){
                deleteContent(item);
            }
        }
        //回收这个文件夹所占有的磁盘空间
        if(item.length!=0){
            uint8_t blockIndex = item.startPos;
            uint8_t next;
            while(blockIndex!=255){
                next = fat[blockIndex];
                fat[blockIndex] = 0;
                blockIndex = next;
            }
        }
    }
    //修改过的fat需要写回磁盘，但不在这里写，在FileOperator::rd中写
    return true;
}

bool FileOperator::writeFile(std::string path, uint8_t *buff, uint32_t length)
{
    /**
      *检查参数是否正确
      *    不正确：（写入失败）
      *    正确：检查文件是否存在
      *         不存在：（写入失败）
      *         存在：找出足够的磁盘空间
      *              空间不够：（写入失败）
      *              空间够：查看文件最后一块盘块是否满
      *                     满或源文件内容为空：从新盘块写起
      *                                      调整fat，并把调整的fat写回磁盘。
      *                                      修改此文件ContentItem的length，
      *                                      若该文件原本内容为空，则还要修改startPos
      *                     未满：从文件所占的最后一块盘块的空闲位置写起
      *                          调整fat，并把调整的fat写回磁盘。
      *
      */
    //检查参数是否正确
    QRegExp reg(QString("^rot(/[^\\$\\./]{3})*(/[^\\$\\./]{3}\\.[^\\$\\./]{2})$"));
    if(!reg.exactMatch(QString(path.data())))
        return false;
    //检查文件是否存在
    Option<ContentItem> result = findContentItem(path);
    if(result.none)
        return false;
    //检查文件是否打开
    if(openedList.find(path)==openedList.end())
        return false; //文件未打开
    ContentItem file = result.some;
    //检查文件是否以写模式打开
    if(openedList.at(path).openModel==FileOperator::READMODEL)
        return false;
    ContentItem modifiedFile = file;
    //检查空间是否够
    {
        uint8_t requiredSize = length;
        if(file.length % 64 != 0){
            requiredSize -= (64 - (file.length % 64));
        }
        uint8_t requiredBlockNum = requiredSize / 64;
        uint8_t count = 0;
        bool haveEnoughBlock = false;
        for(uint8_t blockIndex = 3;blockIndex<128;++blockIndex){
            if(fat[blockIndex]==0)
                ++count;
            if(count==requiredBlockNum){
                haveEnoughBlock = true;
                break;
            }
        }
        if(!haveEnoughBlock)
            return false; //空间不够
        modifiedFile.length += length;
    }
    //写入
    {
        uint32_t buffIndex = 0;
        uint32_t tLength = length;
        uint8_t lastBlockIndex;  //此文件所占随后一个盘块的Index
        uint8_t t = file.startPos;
        if(file.length > 0){
            //文件不为空
            while(t!=255){
                lastBlockIndex = t;
                t = fat[t];
            }
        }
        if(file.length % 64 != 0){
            //文件不为空且最后一块不满
            //找出文件所占的最后一块盘块
            //先写满最后一块盘块
            unsigned surplusSpaceSize = 64 - (file.length % 64); //最后一块剩余容量
            if(length >= surplusSpaceSize){
                uint8_t lastBlockInnerIndex = file.length % 64;
                Block lastBlock = disk.readBlock(lastBlockIndex).some;
                for(unsigned i = 1;i<surplusSpaceSize;++i){
                    lastBlock[lastBlockInnerIndex] = buff[buffIndex];
                    ++lastBlockInnerIndex;
                    ++buffIndex;
                }
                disk.writeBlock(lastBlock,lastBlockIndex);
            }
            tLength -= surplusSpaceSize;
        }
        //把剩下的写到新盘块里
        {
            Block tBlock;
            uint8_t head, tail;
            if(tLength!=0){
                head = tail = findEmptyBlock().some;
            }
            while(tLength!=0){
                uint8_t emptyBlockIndex = findEmptyBlock().some;
                if(tLength >= 64) {
                    for(unsigned i = 0;i<64;++i)
                        tBlock[i] = buff[buffIndex + i];
                    tLength -= 64;
                }else {
                    for(unsigned i =0;i<tLength;++i)
                        tBlock[i] = buff[buffIndex + i];
                    tLength = 0;
                }
                disk.writeBlock(tBlock,emptyBlockIndex); //写入磁盘
                //修改fat
                fat[tail] = emptyBlockIndex;
                fat[emptyBlockIndex] = 255;
                tail = emptyBlockIndex;
            }
            if(file.length==0){
                modifiedFile.startPos = head;
            }else{
                fat[lastBlockIndex] = head;
            }
        }
        //把fat写回磁盘
        {
            Block block;
            for(unsigned i = 0;i<64;++i)
                block[i] = fat[i];
            disk.writeBlock(block,0);
            for(unsigned i = 0;i<64;++i)
                block[i] = fat[64 + i];
            disk.writeBlock(block,1);
        }
        //把修改的此文件的ContentItem写回磁盘
        {
            Block block;
            uint8_t blockIndex,innerIndex;
            std::tie(blockIndex,innerIndex) = findIndex(path);
            block = disk.readBlock(blockIndex).some;
            auto arr = modifiedFile.toUint8Array();
            for(unsigned i = 0;i<8;++i)
                block[innerIndex + i] = arr[i];
            disk.writeBlock(block,blockIndex);
        }
    }
    return true;
}
