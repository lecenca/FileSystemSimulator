#include "textdialog.h"
#include "ui_textdialog.h"

#include <qdebug.h>

TextDialog::TextDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextDialog)
{
    ui->setupUi(this);
}

TextDialog::~TextDialog()
{
    delete ui;
}

void TextDialog::closeEvent(QCloseEvent *event)
{
    if(ui->textEdit->toPlainText().length()!=0){
        /***/
        qInfo()<<"in TextDialog::closeEvent";
        qInfo()<<ui->textEdit->toPlainText();
        /***/
        //把新加的数据写回到文件中
    }
}
