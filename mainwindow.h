#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fileoperator.h"

#include <QMainWindow>
#include <vector>
#include <map>

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
    void intoFolder(std::string folderName);

private slots:

    void on_listWidget_customContextMenuRequested(const QPoint &pos);

    void on_actionCreateNew_triggered();

    void on_actionDeleteFile_triggered();

    void on_actionDeleteFolder_triggered();

    void on_listWidget_doubleClicked(const QModelIndex &index);

    void on_actionBack_triggered();

    void on_writeModelOpen_triggered();

    void on_readModelOpen_triggered();

    void on_actionChangeProperty_triggered();

private:

    Ui::MainWindow *ui;

    std::string seletedItemName; //右键点击选择的项的名字
    FileOperator* fileOperator;
    std::string folderPath;//当前文件夹的路径
    std::vector<ContentItem> currentFolderContent;//当前文件夹的内容
    //std::map<std::string,ContentItem> currentFolderContents;
};

#endif // MAINWINDOW_H
