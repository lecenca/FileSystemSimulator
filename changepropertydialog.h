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
    explicit ChangePropertyDialog(QWidget *parent = 0);
    ~ChangePropertyDialog();

private:
    Ui::ChangePropertyDialog *ui;
};

#endif // CHANGEPROPERTYDIALOG_H
