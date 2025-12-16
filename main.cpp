#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/icons/logo.png"));
    a.setApplicationName("leafix");
    a.setOrganizationName("Demchuk Bohdan");
    a.setApplicationDisplayName("Leafix Editor");
    QFile f(":/styles/app.qss");


    bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);


    if (ok) {
        const QString qss = QString::fromUtf8(f.readAll());
        qApp->setStyleSheet(qss);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
