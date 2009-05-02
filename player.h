#ifndef PLAYER_H_
#define PLAYER_H_
#include <QObject>
#include <QGraphicsItem>
#include "coloritem.h"

class Player:public QObject, public QGraphicsItem
{
	Q_OBJECT

	public:
		Player(QColor color, int width=300, int height=200, QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);
		~Player();
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
		QRectF boundingRect() const;
		bool getSurrendered() const;
		int getScore() const {return score;}
		QColor getColor() const {return color;}
		const QString& getName() const;
		void setName(const QString&);
		ColorItem* getTile(int i) const {return items[i];}
	public slots:
		virtual void startTurn();
		virtual void turnComplete(QColor color,QString tile,int item,int x,int y);
		virtual void surrender();
	signals:
		void turnComplete();
		void scoreChanged(int score);
		void iWin(Player*);
	protected:
		QList<ColorItem*> items;
		virtual void deactivateAll();
		virtual void activateAll();
		int height,width;
		QColor color;
		QString name;
		int score;
		int tilesleft;
		bool surrendered;
		bool active;
		friend class Game;
};
#endif

