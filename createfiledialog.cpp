#include "createfiledialog.h"
#include "ui_createfiledialog.h"

#include <qdebug.h>

CreateFileDialog::CreateFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateFileDialog)
{
    ui->setupUi(this);

    ui->comboBox->addItem(tr("文件"));
    ui->comboBox->addItem(tr("目录"));
}

CreateFileDialog::~CreateFileDialog()
{
    delete ui;
}

void CreateFileDialog::on_OKButton_clicked()
{
    QString name = ui->nameEdit->text();
    QString type = ui->typeEdit->text();
    QString kind = ui->comboBox->currentText();

    if(name.length())

    /***/
    qInfo()<<"in CreateFileDialog::on_OKButton_clicked";
    qInfo()<<name<<type<<kind;
    /***/

    uint8_t property = kind==QString("文件")?0b00000100:0b00001000;
    emit createFile(name.toStdString(),type.toStdString(),property);
    this->close();
}

void CreateFileDialog::on_escButton_clicked()
{
    this->close();
}
