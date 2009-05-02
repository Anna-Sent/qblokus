#include "networkplayer.h"
#include "table.h"
#include <QGraphicsScene>

NetworkPlayer::NetworkPlayer(QColor clr,Table* table,int wid,int hei , QGraphicsItem * parent, QGraphicsScene * scene):Player(clr,wid,hei,parent,scene),tbl(table)
{

}

void NetworkPlayer::turnComplete(QColor color,QString tile,int item,int x,int y)
{
	if (color!=this->color) return;
	Tile til(tile.toStdString());
	Player::turnComplete(color,tile,item,x,y);
	tbl->Accept(x,y,til,item,true,false,color);
}

