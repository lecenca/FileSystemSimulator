#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "createfiledialog.h"
#include "changepropertydialog.h"
#include "textdialog.h"
#include "testopendialog.h"

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    try{
        ui->setupUi(this);
        setFixedSize(this->width(),this->height());
        folderPath = "rot";
        fileOperator = FileOperator::getInstance();
        refreshFolderDisplay();

        /***/
        //ui->listWidget->addAction(ui->actionCreateNew);
        /***/

    }
    catch(std::out_of_range e){
        qInfo()<<"break in MainWindow::MainWindow()";
        qInfo()<<"catch a out_of_range exception in MainWindow::MainWindow";
        throw e;
    }
    catch(std::exception e){
        qInfo()<<"break in MainWindow::MainWindow";
        throw e;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::createFile(std::string name,std::string type,uint8_t property)
{
    bool createSuccrssfully = false;
    std::string path = folderPath + "/" + name;
    if((property & ContentItem::MENU) != ContentItem::MENU){
        //创建文件
        path = path + "." + type;
        createSuccrssfully = fileOperator->createFile(path,property);
    }else{
        //创建目录
        createSuccrssfully = fileOperator->md(path,property);
    }
    //若创建成功，刷新当前文件夹显示
    if(createSuccrssfully)
        refreshFolderDisplay();
}


//重新读取当前文件夹内容，并在列表中显示。
void MainWindow::refreshFolderDisplay()
{
    try{
        ui->pathBar->setText(folderPath.data());
        uint8_t buff[64];
        currentFolderContents.clear();
        if(!(fileOperator->openFile(folderPath,FileOperator::READMODEL))){
            //打开失败
            qInfo()<<"break in MainWindow::refreshFolderDisplay";
            qInfo()<<"open folderPath false, this should not happen\n";
            throw std::exception();
        }
        uint8_t readedCount;
        for(;;){
            readedCount = fileOperator->readFile(folderPath,buff,64);
            if(readedCount==0)
                break;
            for(unsigned i = 0;i<readedCount;i+=8){
                ContentItem item;
                std::string name;
                item = ContentItem::convertToItem(buff+i);
                name.push_back((char)item.name[0]);
                name.push_back((char)item.name[1]);
                name.push_back((char)item.name[2]);
                if((item.property&ContentItem::MENU)!=ContentItem::MENU){
                    name.push_back('.');
                    name.push_back((char)item.type[0]);
                    name.push_back((char)item.type[1]);
                }
                currentFolderContents.insert(std::make_pair(name,item));
            }
        }
        //将currentFolderContent的内容重新显示在listWidge上
        ui->listWidget->clear();
        for(auto iter = currentFolderContents.begin();iter!=currentFolderContents.end();++iter){
            if(iter->first.length()==3){
                ui->listWidget->addItem(
                            new QListWidgetItem(
                                QIcon(":/image/folder.ico"),
                                QString(iter->first.data()),
                                ui->listWidget));
            }else{
                ui->listWidget->addItem(
                            new QListWidgetItem(
                                QIcon(":/image/file.png"),
                                QString(iter->first.data()),
                                ui->listWidget));
            }
        }
        fileOperator->closeFile(folderPath);
    }
    catch(std::exception e){
        qInfo()<<"break in MainWindow::refreshFolderDisplay";
        throw e;
    }
}

//右键点击列表执行该函数
void MainWindow::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = new QMenu(this);
    QListWidgetItem* item = ui->listWidget->itemAt(ui->listWidget->mapFromGlobal(QCursor::pos()));
    if(item==nullptr){
        //右击了菜单空白处
        menu->addAction(ui->actionCreateNew);
    }else{
        seletedItemName = item->text().toStdString();
        if(seletedItemName.length()==6){
            //右键点击了文件
            menu->addAction(ui->readModelOpen);
            menu->addAction(ui->writeModelOpen);
            menu->addAction(ui->actionDeleteFile);
            menu->addAction(ui->actionChangeProperty);
            menu->addAction(ui->actionTestOpen);
        }else{
            //右键点击了目录
            menu->addAction(ui->actionDeleteFolder);
        }
    }
    menu->exec(QCursor::pos());
}

//右键菜单或菜单点击“新建”执行该函数
void MainWindow::on_actionCreateNew_triggered()
{
    CreateFileDialog* createFileDialog = new CreateFileDialog(this);
    connect(createFileDialog,CreateFileDialog::createFile,
            this,MainWindow::createFile);

    createFileDialog->show();
}

//右键菜单点击“删除文件”执行该函数
void MainWindow::on_actionDeleteFile_triggered()
{
    fileOperator->deleteFile(folderPath+"/"+seletedItemName);
    refreshFolderDisplay();

}

//右键菜单点击“删除目录”执行该函数
void MainWindow::on_actionDeleteFolder_triggered()
{
    fileOperator->rd(folderPath+"/"+seletedItemName);
    refreshFolderDisplay();
}

//双击item执行该函数，双击文件时，以制度模式打开，双击目录时，进入该目录
void MainWindow::on_listWidget_doubleClicked(const QModelIndex &index)
{
    //现在只实现双击打开文件夹的功能
    QListWidgetItem* item = ui->listWidget->item(index.row());
    if(item->text().length()==3){
        //双击了文件夹，则进入该文件夹
        intoFolder(item->text().toStdString());
    }else{
        //双击了文件，则以只读模式打开文件
        if(!fileOperator->openFile(folderPath + "/" + item->text().toStdString(),FileOperator::READMODEL)){
            //以读模式打开只读文件，打开失败
            return;
        }
        //若打开成功，先把文件关掉
        fileOperator->closeFile(folderPath + "/" + item->text().toStdString());
        TextDialog* textDialog = new TextDialog(folderPath + "/" + item->text().toStdString(),
                                                FileOperator::READMODEL,
                                                this);
        textDialog->show();
    }
}

void MainWindow::intoFolder(std::string folderName)
{
    folderPath = folderPath + "/" + folderName;
    refreshFolderDisplay();
}

//单击 “后退” 执行该函数
void MainWindow::on_actionBack_triggered()
{
    if(folderPath!="rot"){
        folderPath = folderPath.substr(0,folderPath.length()-4);
        refreshFolderDisplay();
    }
}

//点击右键菜单中的 “以写模式打开” 运行此函数
void MainWindow::on_writeModelOpen_triggered()
{

    if(!fileOperator->openFile(folderPath + "/" + seletedItemName,FileOperator::WRITEMODEL)){
        //以写模式打开只读文件，打开失败
        return;
    }
    //若打开成功，先把文件关掉
    fileOperator->closeFile(folderPath + "/" + seletedItemName);
    TextDialog* textDialog = new TextDialog(folderPath + "/" + seletedItemName,
                                            FileOperator::WRITEMODEL,
                                            this);
    textDialog->show();
}

//点击右键菜单中的 “以读模式打开” 运行此函数
void MainWindow::on_readModelOpen_triggered()
{
    if(!fileOperator->openFile(folderPath + "/" + seletedItemName,FileOperator::READMODEL)){
        //以读模式打开只读文件，打开失败
        return;
    }
    //若打开成功，先把文件关掉
    fileOperator->closeFile(folderPath + "/" + seletedItemName);
    TextDialog* textDialog = new TextDialog(folderPath + "/" + seletedItemName,
                                            FileOperator::READMODEL,
                                            this);
    textDialog->show();
}

//点击右键菜单“修改属性”运行此函数
void MainWindow::on_actionChangeProperty_triggered()
{
    auto item = currentFolderContents.at(seletedItemName);
    ChangePropertyDialog *changePropertyDialog = new ChangePropertyDialog(seletedItemName,item.property,this);
    connect(changePropertyDialog,ChangePropertyDialog::changeProperty,
                this,MainWindow::changeProperty);
    changePropertyDialog->show();

}

//修改属性
void MainWindow::changeProperty(std::string name, uint8_t property)
{
    std::string path;
    path = folderPath + "/" + name;
    fileOperator->change(path,property);
    refreshFolderDisplay();
}

//点击右键菜单“测试性打开”运行此函数
void MainWindow::on_actionTestOpen_triggered()
{
    std::string path = folderPath + "/" + seletedItemName;
    if(!fileOperator->openFile(path,FileOperator::READMODEL))
        //若打开失败，则返回
        return;
    //若成功，则先关掉文件，再交给TestOpenDialog处理
    fileOperator->closeFile(path);
    TestOpenDialog* testOpenDialog = new TestOpenDialog(path,this);
    testOpenDialog->show();
}
