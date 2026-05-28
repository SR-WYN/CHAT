#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <qcoreapplication.h>
#include <qobject.h>
#include <qdir.h>
#include <qfile.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qss(":/res/style/style.qss");
    if (qss.open(QFile::ReadOnly))
    {
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }
    else
    {
    }
    
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
