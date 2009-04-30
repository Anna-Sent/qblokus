#include <QApplication>
#include "app.h"
#include "game.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    App *dialog = new App;
//    Game game(dialog);
//    game.addPlayer(QString::fromUtf8("fara"),Qt::green,ptLocal);
//    game.addPlayer(QString::fromUtf8("anna"),Qt::red,ptNetwork);
//    game.start();
    return app.exec();
}
