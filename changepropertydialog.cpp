#include "changepropertydialog.h"
#include "ui_changepropertydialog.h"

ChangePropertyDialog::ChangePropertyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePropertyDialog)
{
    ui->setupUi(this);
}

ChangePropertyDialog::~ChangePropertyDialog()
{
    delete ui;
}
