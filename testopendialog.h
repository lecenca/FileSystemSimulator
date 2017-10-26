#ifndef TESTOPENDIALOG_H
#define TESTOPENDIALOG_H

#include <QDialog>

namespace Ui {
class TestOpenDialog;
}

class TestOpenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestOpenDialog(std::string path, QWidget *parent = 0);
    ~TestOpenDialog();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::TestOpenDialog *ui;

    std::string path;
};

#endif // TESTOPENDIALOG_H
