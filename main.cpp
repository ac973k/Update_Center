#include "updatecenter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UpdateCenter w;
    w.show();
    return a.exec();
}
