#include "testopendialog.h"
#include "ui_testopendialog.h"
#include "fileoperator.h"

TestOpenDialog::TestOpenDialog(std::string path, QWidget *parent) :
    path(path),
    QDialog(parent),
    ui(new Ui::TestOpenDialog)
{
    ui->setupUi(this);

    FileOperator::getInstance()->openFile(path,FileOperator::READMODEL);
}

TestOpenDialog::~TestOpenDialog()
{
    delete ui;
}


void TestOpenDialog::closeEvent(QCloseEvent *event)
{
    FileOperator::getInstance()->closeFile(path);
}
