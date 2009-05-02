#include "table.h"

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QPainter>
#include "tile.h"
#include <string>
#include <QStringList>

typedef std::pair<int,QString> intstr_t;

intstr_t dataparse(QString what)
{
	QStringList parts = what.split(QChar(':'));
	return std::make_pair(parts[0].toInt(),parts[1]);
}


TableCell::TableCell(QGraphicsItem *parent,int x,int y)
: QGraphicsItem(parent), color(Qt::lightGray), dragOver(false), xpos(x), ypos(y)
{
	setAcceptDrops(true);
}

void TableCell::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
	bool result=false;
	if (event->mimeData()->hasText())
	{
		QString text = event->mimeData()->text();
		intstr_t data = dataparse(text);
		Tile tile(data.second.toStdString());
		Table* table = qgraphicsitem_cast<Table*>(parentItem());
		if (table)
		{
			QColor color = qVariantValue<QColor>(event->mimeData()->hasColor()?event->mimeData()->colorData():QColor(0,0,0));
			result = table->Accept(xpos,ypos,tile,data.first,false,true,color);
		}
	}
	if (result)
	{
		event->setAccepted(true);
	} else {
		event->setAccepted(false);
	}
}

void TableCell::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
	Q_UNUSED(event);
//	dragOver = false;
/*	Table* table = qgraphicsitem_cast<Table*>(parentItem());
	if (table) table->clearDrags();*/
//	update();
}

void TableCell::dropEvent(QGraphicsSceneDragDropEvent *event)
{
	bool result=false;
	if (event->mimeData()->hasText())
	{
		QString text = event->mimeData()->text();
		intstr_t data = dataparse(text);
		Tile tile(data.second.toStdString());
		Table* table = qgraphicsitem_cast<Table*>(parentItem());
		if (table)
		{
			QColor color = qVariantValue<QColor>(event->mimeData()->hasColor()?event->mimeData()->colorData():QColor(0,0,0));
			result = table->Accept(xpos,ypos,tile,data.first,true,true,color);
		}
	}
	if (result)
	{
		event->setAccepted(true);
	} else {
		event->setAccepted(false);
	}
}
void TableCell::paint(QPainter *painter,
		const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->fillRect(1,1,18,18,dragOver ? color.light(130) : color);

}
QRectF TableCell::boundingRect() const
{
	return QRectF(0, 0, 20, 20);
}

Table::Table(int w,int h) : width(w),height(h)
{
	cells.resize(h);
	for(int i = 0;i<h;++i)
	{
		cells[i].resize(w);
		for(int j=0;j<w;++j)
		{
			TableCell * ne = new TableCell(this,j,i);
			ne->setPos(j*20,i*20);
			cells[i][j]=ne;
		}
	}
}

QRectF Table::boundingRect() const
{
	return QRectF(0, 0, width*20, height*20);
}

void Table::paint(QPainter *painter,
		const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	Q_UNUSED(painter);
	painter->fillRect(0,0,width*20,height*20,Qt::darkGray);
}

std::string readColor(QColor color)
{
	QString res = QString("QColor(%1, %2, %3)\n")
                .arg(color.red()).arg(color.green()).arg(color.blue());
	return res.toStdString();
}

bool Table::Accept(int x,int y,const Tile& what,int id,bool really,bool local,QColor color)
//bool Table::Accept(int x,int y,Tile what,bool really,QColor color)

{
	if (x+what.getWidth()>width) return false;
	if (y+what.getHeight()>height) return false;
	clearDrags();
//	std::cout << "accepting...\n";
	bool inAngle=false;
	bool touchAngles=false;
	bool valid=true;
	for(int i=x;i<x+what.getWidth();++i)
	{
		for(int j=y;j<y+what.getHeight();++j)
		{
			if (what.data[j-y][i-x]!='0')
			{
				if (really)
				{
					cells[j][i]->color=color;
				}
				else
				{
					cells[j][i]->dragOver=true;
					if (cells[j][i]->color!=Qt::lightGray) return false;
//					std::cout << j << " " << height << "\n";
					if (j>0)
					{
						if (cells[j-1][i]->color==color) valid=false;
					}
					if (j<height-1)
					{
						if (cells[j+1][i]->color==color) valid=false;
					}
					if (i>0)
					{
						if (cells[j][i-1]->color==color) valid=false;
					}
					if (i<width-1)
					{
						if (cells[j][i+1]->color==color) valid=false;
					}
					if (j>0&&i>0)
					{
						if (cells[j-1][i-1]->color==color) touchAngles=true;
					}
					if (j<height-1&&i>0)
					{
						if (cells[j+1][i-1]->color==color) touchAngles=true;
					}
					if (j>0&&i<width-1)
					{
						if (cells[j-1][i+1]->color==color) touchAngles=true;
					}
					if (j<height-1&&i<width-1)
					{
						if (cells[j+1][i+1]->color==color) touchAngles=true;
					}
					if ((i==0&&j==0)||(i==width-1&&j==height-1)||(j==0&&i==width-1)||(i==0&&j==height-1))
						inAngle=true;

//					std::cout << valid << "\n";
					if (!valid) return false;
				
				}
				cells[j][i]->update();
			}
		}
	}
//	std::cout << "!accepting\n";
	if (really&&local)
	{
		emit turnComplete(color,what.getAsQString(),id,x,y);
	}
	if (really) std::cerr << what << std::endl;

	if (inAngle||touchAngles)
	{
		return true;
	}
	else return false;
}

void Table::clearDrags()
{
//	std::cout << "clear\n";
	for(int i=0;i<width;++i)
		for(int j=0;j<height;++j)
		{
			cells[i][j]->dragOver=false;
			cells[i][j]->update();
//			std::cout << i << " " << j << "\n";		
		}

}

