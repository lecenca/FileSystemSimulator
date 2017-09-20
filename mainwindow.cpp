#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "createfiledialog.h"

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    try{
        ui->setupUi(this);
        setFixedSize(this->width(),this->height());
        folderPath = "rot";
        refreshFolderDisplay();

        /***/
        ui->listWidget->addAction(ui->actionCreateNew);
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
    if((property & 0b00000100) == NORMAL){
        //创建文件
        path = path + "." + type;
        createSuccrssfully = fileOperator.createFile(path,property);
    }else{
        //创建目录
        createSuccrssfully = fileOperator.md(path,property);
    }
    //若创建成功，刷新当前文件夹显示
    if(createSuccrssfully)
        refreshFolderDisplay();
}


//重新读取当前文件夹内容，并在列表中显示。
void MainWindow::refreshFolderDisplay()
{
    try{
        ui->pathLabel->setText(folderPath.data());
        uint8_t buff[64];
        uint8_t readedCount;
        currentFolderContent.clear();
        if(!fileOperator.openFile(folderPath,0)){
            //打开失败
            qInfo()<<"break in MainWindow::refreshFolderDisplay";
            qInfo()<<"open folderPath false, this should not happen";
            throw std::exception();
        }
        for(;;){
            //把当前文件夹内容放入currentFolderContent
            readedCount = fileOperator.readFile(folderPath,buff,64);
            if(readedCount==0)
                break;
            for(unsigned i = 0;i<readedCount;i+=8){
                currentFolderContent.push_back(ContentItem::convertToItem(buff+i));
            }
            /***/
            qInfo()<<"in MainWindow::refreshFolderDisplay";
            qInfo()<<"readCount ="<<readedCount;
            /***/
        }
        //将currentFolderContent的内容重新显示在listWidge上
        ui->listWidget->clear();
        for(ContentItem item: currentFolderContent){
            if((item.property & 0b00001000) == 0b00001000){
                ui->listWidget->addItem(
                            new QListWidgetItem(
                                QIcon(":/image/folder.ico"),
                                QString(QByteArray((char*)item.name,3)),
                                ui->listWidget));
            }else{
                ui->listWidget->addItem(
                            new QListWidgetItem(
                                QIcon(":/image/file.png"),
                                QString(QByteArray((char*)item.name,3)+"."+QByteArray((char*)item.type,2)),
                                ui->listWidget));
            }
        }
        fileOperator.closeFile(folderPath);
    }
    catch(std::out_of_range e){
        qInfo()<<"break in MainWindow::refreshFolderDisplay";
        qInfo()<<"catch a out_of_range exception in MainWindow::refreshFolderDisplay";
        throw e;
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
        menu->addAction(ui->actionCreateNew);
    }else{
        seletedItemName = item->text().toStdString();
        QRegExp reg(QString("^([^\\$\\./]{3})\\.(([^\\$\\./]){2})$"));
        QString name = item->text();
        if(reg.exactMatch(name)){
            menu->addAction(ui->actionDeleteFile);
        }else{
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

bool MainWindow::deleteFile(std::string path)
{
    fileOperator.deleteFile(path);
}

//右键菜单点击“删除”执行该函数
void MainWindow::on_actionDeleteFile_triggered()
{
    deleteFile(folderPath+"/"+seletedItemName);
    refreshFolderDisplay();

}
