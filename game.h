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
		void clear();
		bool isStarted() const;
		void remotePlayerRetired(QString name,QColor color);
	
	public slots:
		void start();
		void turnDone(QColor color, QString tile,int id,int x,int y);
		void addPlayer(QString name,QColor color, PlayerType type);
		void playerRetired();
	private slots:
		void winner(Player*);
	signals:
		void gameOver(QString winner,int score,QColor color);
		void playerRetired(QString name, QColor color);
		void turnDone(QString name,QColor color, QString mask,int id,int x,int y);
	protected:
		QList<Player*> players;
		QList<QGraphicsScene*> scenes;
		QGraphicsScene* tablescene;
		Table* table;
		Ui::MainWindow* ui;
		QWidget* widget;
		int currplayer;
		bool running;
		int playersleft;
		bool isserver;
		void retirePlayer(int);
};
