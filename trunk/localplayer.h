#include "player.h"

class LocalPlayer:public Player
{
	Q_OBJECT
	public:
		LocalPlayer(QColor clr,int wid=200,int hei=200 , QGraphicsItem * parent=0, QGraphicsScene * scene=0);
};

