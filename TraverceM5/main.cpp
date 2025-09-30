#include "traverce.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Traverce w;
    w.show();
    return a.exec();
}
