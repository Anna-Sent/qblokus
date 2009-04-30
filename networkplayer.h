#include "player.h"

class NetworkPlayer:public Player
{
	Q_OBJECT
	public:
		NetworkPlayer(QColor clr,int wid=200,int hei=200 , QGraphicsItem * parent=0, QGraphicsScene * scene=0);
	protected:
		void deactivateAll() {active=false;update();};
		void activateAll() {active=true;update();};
};

