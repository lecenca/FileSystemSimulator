#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fileoperator.h"

#include <QMainWindow>
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void createFile(std::string name,std::string type,uint8_t property);
    void refreshFolderDisplay();
    std::string getCurrentFolderPath();
    bool deleteFile(std::string path);

private slots:

    void on_listWidget_customContextMenuRequested(const QPoint &pos);

    void on_actionCreateNew_triggered();

    void on_actionDeleteFile_triggered();

private:
    static const uint8_t MENU = 0b00001000;
    static const uint8_t NORMAL = 0b00000100;
    static const uint8_t SYSTEM = 0b00000010;
    static const uint8_t READONLY = 0b00000001;

    Ui::MainWindow *ui;

    std::string seletedItemName; //右键点击选择的项的名字
    FileOperator fileOperator;
    std::string folderPath;//当前文件夹的路径
    std::vector<ContentItem> currentFolderContent;//当前文件夹的内容
};

#endif // MAINWINDOW_H
