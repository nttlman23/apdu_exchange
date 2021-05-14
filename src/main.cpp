#include "mainwindow.h"
#include <QApplication>
#include <QPalette>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    Q_INIT_RESOURCE(test_ex);

    QApplication a(argc, argv);

#if _WIN32 || _WIN64
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
#endif
//    QFont font;
//    font.setFamily(font.defaultFamily());
//    a.setFont(font);

//    QTranslator myTranslator;
//    myTranslator.load("i18n/test_ex_" + QLocale::system().name());
//    a.installTranslator(&myTranslator);

    MainWindow w;
    w.show();

    return a.exec();
}
