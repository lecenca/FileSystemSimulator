#include "textdialog.h"
#include "ui_textdialog.h"

#include <qdebug.h>

TextDialog::TextDialog(std::string path, uint8_t openModel, QWidget *parent) :
    QDialog(parent),path(path),openModel(openModel),
    ui(new Ui::TextDialog)
{
    ui->setupUi(this);
    fileOperator = FileOperator::getInstance();
    /***/
    qInfo()<<"in TextDialog::TextDialog";
    qInfo()<<"001\n";
    /***/
    if(openModel==FileOperator::READMODEL){
        /***/
        qInfo()<<"in TextDialog::TextDialog";
        qInfo()<<"002\n";
        /***/
        fileOperator->openFile(path,openModel);
        //显示内容
        uint32_t readCount;
        uint8_t buff[64];
        QString data;
        for(;;){
            readCount = fileOperator->readFile(path,buff,64);
            if(readCount==0)
                break;
            QString tStr = QString::fromLocal8Bit(reinterpret_cast<char*>(buff),readCount);
            data += tStr;
            /***/
            qInfo()<<"in TextDialog::TextDialog";
            qInfo()<<"readCount: "<<readCount<<"\n";
            /***/
        }
        /***/
        qInfo()<<"in TextDialog::TextDialog";
        qInfo()<<"data: "<<data<<"\n";
        /***/
        ui->textEdit->setText(data);
        fileOperator->closeFile(path);
    }
}

TextDialog::~TextDialog()
{
    delete ui;
}

void TextDialog::closeEvent(QCloseEvent *event)
{
    if(ui->textEdit->toPlainText().length()!=0 && openModel==FileOperator::WRITEMODEL){
        /***/
        qInfo()<<"in TextDialog::closeEvent";
        qInfo()<<ui->textEdit->toPlainText();
        /***/
        //把新加的数据写回到文件中
        std::string data = ui->textEdit->toPlainText().toStdString();
        if(openModel==FileOperator::WRITEMODEL){
            fileOperator->openFile(path,openModel);
            fileOperator->writeFile(path,
                                    (uint8_t*)(data.data()),
                                    data.length()*sizeof(decltype(*(data.data())))/sizeof(uint8_t));
            fileOperator->closeFile(path);
        }   
    }
}
