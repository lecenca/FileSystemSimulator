#include "filelistwidget.h"

#include <qmenu.h>

#include <qdebug.h>

FileListWidget::FileListWidget(QWidget *parent):
    QListWidget(parent)
{
    createNew = new QAction(tr("新建"),this);
    deleteFile = new QAction(tr("删除文件"),this);
    deleteFolder = new QAction(tr("删除目录"),this);
}
