#ifndef CHANGEPROPERTYDIALOG_H
#define CHANGEPROPERTYDIALOG_H

#include <QDialog>

namespace Ui {
class ChangePropertyDialog;
}

class ChangePropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePropertyDialog(std::string name, uint8_t property, QWidget *parent = 0);
    ~ChangePropertyDialog();

signals:
    void changeProperty(std::string name,uint8_t property);

private slots:
    void on_OKButton_clicked();

    void on_discard_clicked();

private:
    Ui::ChangePropertyDialog *ui;

    std::string fileName;
    uint8_t property;

};

#endif // CHANGEPROPERTYDIALOG_H
