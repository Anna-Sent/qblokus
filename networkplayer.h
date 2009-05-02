#include "player.h"
class Table;

class NetworkPlayer:public Player
{
	Q_OBJECT
	public:
		NetworkPlayer(QColor clr,Table* table,int wid=200,int hei=200 , QGraphicsItem * parent=0, QGraphicsScene * scene=0);
	public slots:
		void turnComplete(QColor color,int item,int x,int y);
	protected:
		void deactivateAll() {active=false;update();};
		void activateAll() {active=true;update();};
		Table* tbl;
};

