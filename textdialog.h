#ifndef TEXTDIALOG_H
#define TEXTDIALOG_H

#include "fileoperator.h"

#include <string>

#include <QDialog>

namespace Ui {
class TextDialog;
}

class TextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextDialog(std::string path, uint8_t openModel, QWidget *parent = 0);
    ~TextDialog();

protected:
    void closeEvent(QCloseEvent *event);

private:
    FileOperator* fileOperator;
    std::string path;
    uint8_t openModel;
    Ui::TextDialog *ui;
};

#endif // TEXTDIALOG_H
