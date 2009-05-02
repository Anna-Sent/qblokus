#include "networkplayer.h"
#include "table.h"
#include <QGraphicsScene>

NetworkPlayer::NetworkPlayer(QColor clr,Table* table,int wid,int hei , QGraphicsItem * parent, QGraphicsScene * scene):Player(clr,wid,hei,parent,scene),tbl(table)
{

}

void NetworkPlayer::turnComplete(QColor color,int item,int x,int y)
{
	if (color!=this->color) return;
	Player::turnComplete(color,item,x,y);
	tbl->Accept(x,y,*items[item],item,true,false,color);
}

