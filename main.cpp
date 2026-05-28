#include "ConfigMgr.h"
#include "Log.h"
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
    ConfigMgr::getInstance();
    if (!Log::init("CHAT", ConfigMgr::getInstance().getLogConfig()))
    {
        return 1;
    }
    Log::info(LogModule::App, "CHAT client starting");
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
    const int code = QCoreApplication::exec();
    Log::info(LogModule::App, "CHAT client exiting with code {}", code);
    Log::shutdown();
    return code;
}
