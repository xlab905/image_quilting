#include <QApplication>
#include <QtWidgets>
#include <io.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //ImageQuilting w;
    IO w;
    w.show();

    return a.exec();
}
