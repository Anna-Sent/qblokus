 /****************************************************************************
 **
 ** Copyright (C) 2006-2008 Trolltech ASA. All rights reserved.
 **
 ** This file is part of the documentation of the Qt Toolkit.
 **
 ** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

 #include <QtGui>
#include <iostream>

 #include "coloritem.h"

/* ColorItem::ColorItem()
     :Tile("111|101|111"), color(qrand() % 256, qrand() % 256, qrand() % 256)
 {
	setToolTip(QString("QColor(%1, %2, %3)\n%4")
                .arg(color.red()).arg(color.green()).arg(color.blue())
                .arg("Click and drag this color onto the robot!"));
     setCursor(Qt::OpenHandCursor);
 }
 ColorItem::ColorItem(std::string mask)
	:Tile(mask), color(qrand() % 256, qrand() % 256, qrand() % 256)

{
	setToolTip(QString("QColor(%1, %2, %3)\n%4")
                .arg(color.red()).arg(color.green()).arg(color.blue())
                .arg("Click and drag this color onto the robot!"));
     setCursor(Qt::OpenHandCursor);
}
*/
 ColorItem::ColorItem(std::string mask, QColor clr,int id)
	:Tile(mask), color(clr), Id(id),active(false)

{
	setToolTip(QString("QColor(%1, %2, %3)\n%4")
                .arg(color.red()).arg(color.green()).arg(color.blue())
                .arg("Click and drag this color onto the robot!"));
     setCursor(Qt::OpenHandCursor);
}

/*
 ColorItem::ColorItem(const char * mask)
	:Tile(std::string(mask)), color(qrand() % 256, qrand() % 256, qrand() % 256)

{
	setToolTip(QString("QColor(%1, %2, %3)\n%4")
                .arg(color.red()).arg(color.green()).arg(color.blue())
                .arg("Click and drag this color onto the robot!"));
     setCursor(Qt::OpenHandCursor);
}
*/
 QRectF ColorItem::boundingRect() const
 {
     //return QRectF(-15.5, -15.5, 34, 34);
	 //std::cout << "getbrect" << getHeight()<< " " << getWidth() <<"\n";
 	 return QRectF(0,0,getWidth()*20,getHeight()*20);
 }

 void ColorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
     Q_UNUSED(option);
     Q_UNUSED(widget);
//     std::cout << "paint" << getHeight()<< " " << getWidth() <<"\n";
/*     painter->setPen(Qt::NoPen);
     painter->setBrush(Qt::darkGray);
     painter->drawEllipse(-12, -12, 30, 30);
     painter->setPen(QPen(Qt::black, 1));
     painter->setBrush(QBrush(color));
     painter->drawEllipse(-15, -15, 30, 30);*/
     painter->setPen(QPen(Qt::black, 1));
     painter->setBrush(QBrush(color));
     int h,s,v;
     color.getHsv(&h,&s,&v);
     QColor clr = color;
     if (!active)
     {
	     clr.setHsv(h,s/2,v);
     }
     for(int i=0;i<getHeight();++i)
     {
	     for(int j=0;j<getWidth();++j)
	     {
//		     std::cout <<  i << " " << j << "\n";
		     if (Tile::data[i][j]!='0') painter->fillRect(j*20+1,i*20+1,18,18,clr);
	     }
     }
 }

 void ColorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
 {
/*     if (event->button() != Qt::LeftButton) {
         event->ignore();
         return;
     }*/
	 if (!active) return;
	 switch(event->button()) {
		 case(Qt::LeftButton): 
		     setCursor(Qt::ClosedHandCursor);
		     break;
		 case(Qt::RightButton):
		     prepareGeometryChange(); 
		     rotateTile();
		     update();
		     break;
		 default:
		     event->ignore();
		     break;
	 }
 }
void ColorItem::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
	if (!active) return;
	Q_UNUSED(event);
//	if (event->button()!=Qt::RightButton) return;
	reflectTile();
	update();
}

 void ColorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
 {
	 if (!active) return;
     if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
         .length() < QApplication::startDragDistance()) {
         return;
     }

     QDrag *drag = new QDrag(event->widget());
     QMimeData *mime = new QMimeData;
     drag->setMimeData(mime);

         mime->setColorData(color);
         mime->setText(QString::number(Id)+QString(':')+QString(getAsText().c_str()));

         QPixmap pixmap(getWidth()*20, getHeight()*20);
         pixmap.fill(Qt::white);

         QPainter painter(&pixmap);
         //painter.translate(15, 15);
         painter.setRenderHint(QPainter::Antialiasing);
         paint(&painter, 0, 0);
         painter.end();

         pixmap.setMask(pixmap.createHeuristicMask());

         drag->setPixmap(pixmap);
         drag->setHotSpot(QPoint(5, 5));

     drag->exec();
     setCursor(Qt::OpenHandCursor);
 }

 void ColorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
 {
     setCursor(Qt::OpenHandCursor);
 }

void ColorItem::activate()
{
	active=true;
}
void ColorItem::deactivate()
{
	active=false;
}

#ifdef DEBUG_DESTRUCTORS
ColorItem::~ColorItem()
		{
			std::cerr << "colorItem " << Id << color.name().toStdString() << " destroyed" << std::endl;
		}
#endif

