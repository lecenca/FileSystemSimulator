#ifndef CREATEFILEDIALOG_H
#define CREATEFILEDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
class CreateFileDialog;
}

class CreateFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateFileDialog(QWidget *parent = 0);
    ~CreateFileDialog();

signals:
    void createFile(std::string name, std::string type, uint8_t property);

private slots:
    void on_OKButton_clicked();

    void on_escButton_clicked();

private:
    Ui::CreateFileDialog *ui;
};

#endif // CREATEFILEDIALOG_H
