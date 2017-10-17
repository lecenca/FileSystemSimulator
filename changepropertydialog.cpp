#include "changepropertydialog.h"
#include "contentitem.h"
#include "ui_changepropertydialog.h"

ChangePropertyDialog::ChangePropertyDialog(std::string name, uint8_t property, QWidget *parent) :
    QDialog(parent),
    fileName(name),
    property(property),
    ui(new Ui::ChangePropertyDialog)
{
    ui->setupUi(this);
    if((property&ContentItem::NORMAL)==ContentItem::NORMAL)
        ui->normal->setChecked(true);
    if((property&ContentItem::SYSTEM)==ContentItem::SYSTEM)
        ui->system->setChecked(true);
    if((property&ContentItem::READONLY)==ContentItem::READONLY)
        ui->readOnly->setChecked(true);

}

ChangePropertyDialog::~ChangePropertyDialog()
{
    delete ui;
}

void ChangePropertyDialog::on_OKButton_clicked()
{
    uint8_t tProperty = 0;
    if(ui->normal->isChecked())
        tProperty = tProperty|ContentItem::NORMAL;
    if(ui->system->isChecked())
        tProperty = tProperty|ContentItem::SYSTEM;
    if(ui->readOnly->isChecked())
        tProperty = tProperty|ContentItem::READONLY;

    if(tProperty != property){
        emit changeProperty(fileName,tProperty);
    }
    this->close();
}

void ChangePropertyDialog::on_discard_clicked()
{
    this->close();
}
