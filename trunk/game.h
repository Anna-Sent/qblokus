#include <QObject>
#include <QList>
#include <QColor>
//#include "clientinfo.h"
class ClientInfo;
class Table;
class Player;
class QGraphicsScene;
namespace Ui
{
	class MainWindow;
}

enum PlayerType {ptLocal,ptNetwork};

class Game:public QObject
{
	Q_OBJECT
	public:
		Game(QWidget* widget);
		~Game();
		void updatePlayers(QList<ClientInfo>,QList<bool>);
		//void clear();
	public slots:
		void start();
		void turnDone(QColor color, int id,int x,int y);
		void addPlayer(QString name,QColor color, PlayerType type);

		void playerRetired();
		void winner(Player*);
	signals:
		void gameOver(QString winner,int score,QColor color);
		void playerRetired(QString name, QColor color);
		void turnDone(QString name,QColor color, int id,int x,int y);
	protected:
		QList<Player*> players;
		QList<QGraphicsScene*> scenes;
		Table* table;
		Ui::MainWindow* ui;
		QWidget* widget;
		int currplayer;
		bool running;
		int playersleft;
};
