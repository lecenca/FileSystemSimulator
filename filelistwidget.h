#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <qlistwidget.h>

class FileListWidget : public QListWidget
{
public:
    FileListWidget(QWidget* parent);

//protected:
//    void contextMenuEvent(QContextMenuEvent* event);

private:

    QAction* deleteFile;
    QAction* deleteFolder;
    QAction* createNew;

};

#endif // FILELISTWIDGET_H
