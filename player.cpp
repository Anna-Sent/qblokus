#include "player.h"
#include <QGraphicsScene>
#include <QPainter>
#include <iostream>

Player::Player(QColor clr,int wid,int hei , QGraphicsItem * parent, QGraphicsScene * scene):QGraphicsItem(parent,scene), height(wid),width(hei),color(clr),name(""),score(0),surrendered(false), active(false)
{

	char const* cTiles[] = { "1", "11", "11|01","111","11|11","010|111","1111","001|111","011|110","1000|1111","010|010|111","100|100|111","0111|1100","001|111|100","1|1|1|1|1","10|11|11","011|110|100","11|10|11","011|110|010","010|111|010","0100|1111"};

	std::vector<std::string> tiles(cTiles,cTiles+21);
	tilesleft=tiles.size();

	int xs=10;
	int ys=10;
	int maxheight=0;
	double dscale=0.5;
	int realwidth=0;
	int realheight=0;

     	for (size_t i = 0; i < tiles.size(); ++i) {
        	ColorItem *item = new ColorItem(tiles[i],color,i);
		items.append(item);
		item->setParentItem(this);
		item->setPos(xs,ys);
		int dim = std::max(item->getHeight(),item->getWidth());
		xs+=dim*20*dscale+10;
		int height=(dim*20*dscale+10);
		item->scale(dscale,dscale);
		if (height>maxheight)
			maxheight=height;
		if (xs>realwidth) realwidth=xs;
		if (xs>width)
		{
			ys+=maxheight;
			xs=10;
			realheight+=maxheight;
			maxheight=0;
		}
	}
	realheight+=maxheight;
	height=realheight;
	width=realwidth;
}

Player::~Player()
{
/*	for(int i=0;i<items.size();++i)
	{
		delete items[i];
	}*/
}

QRectF Player::boundingRect() const 
{
	return QRectF(0,0,width,height);
}

bool Player::getSurrendered() const
{
	return surrendered;
}

void Player::startTurn()
{
	if (surrendered||tilesleft==0)
	{
		emit turnComplete();
		return;
	}
	activateAll();
}

void Player::deactivateAll()
{
	for(int i=0;i<items.size();++i)
	{
		items[i]->deactivate();
	}
	active=false;
	update();
}
void Player::activateAll()
{
	for(int i=0;i<items.size();++i)
	{
		items[i]->activate();
	}
	active=true;
	update();

}

void Player::paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget)
{
	if (surrendered)
		painter->drawText(boundingRect(),Qt::AlignCenter,QString::fromUtf8("Игрок сдался"));
	if (tilesleft==0)
		painter->drawText(boundingRect(),Qt::AlignCenter,QString::fromUtf8("Игрок выиграл!!"));
	Q_UNUSED(style);
	Q_UNUSED(widget);
}

const QString& Player::getName() const
{
	return name;
}
void Player::setName(const QString& newname)
{
	name = newname;
}

void Player::turnComplete(QColor color,int item,int x,int y)
{
	Q_UNUSED(x);
	Q_UNUSED(y);
	--tilesleft;
	if (color!=this->color) return;
	items[item]->hide();
	Tile* tile = items[item];
	score+=tile->score();
	emit scoreChanged(score);
	emit turnComplete();
	deactivateAll();
	if (tilesleft==0)
	{
		emit iWin(this);
	}
}

void Player::surrender()
{
	if (active)
	{
		surrendered=true;
		deactivateAll();
		emit turnComplete();
	}
}

