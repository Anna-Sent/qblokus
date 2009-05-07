#include <QApplication>
#include "app.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    App *dialog = new App;
    return app.exec();
}