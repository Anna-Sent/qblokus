#include "game.h"
#include <QColor>
#include <QGraphicsScene>
#include "table.h"
#include "localplayer.h"
#include "networkplayer.h"
#include "ui_racingForm.h"
#include <iostream>
#include <QColor>
/*
Game::Game(QWidget* widget):currplayer(0)
{
	ui = new Ui::BlokusUi;
	ui->setupUi(widget);
	QList<QColor> colors;
	colors.push_back(QColor(244,120,12));
	colors.push_back(QColor(10,120,250));
	colors.push_back(QColor(11,120,12));
	colors.push_back(QColor(10,240,250));
	table = new Table(20,20);
	QGraphicsScene* tablescene = new QGraphicsScene;
	scenes.append(tablescene);
	tablescene->addItem(table);
	ui->gvTable->setScene(tablescene);
	QString playerwidget("gvPlayer");
	QString playerscore("score");

	for(int i=0;i<colors.size();++i)
	{
		Player* player = new LocalPlayer(colors[i]);
		player->setPos(0,0);
		player->setName(QString("player")+QString::number(i+1));
		players.append(player);
		QGraphicsScene* playerscene = new QGraphicsScene;
		scenes.append(playerscene);
		playerscene->addItem(player);
		QGraphicsView *gv = widget->findChild<QGraphicsView*>(playerwidget+QString::number(i+1));
		QLCDNumber *lcd = widget->findChild<QLCDNumber*>(playerscore+QString::number(i+1));
	
		gv->setScene(playerscene);
		connect(player,SIGNAL(scoreChanged(int)),lcd,SLOT(display(int)));
		connect(player,SIGNAL(iWin(Player* )),this,SLOT(winner(Player *)));
	}
	connect(table,SIGNAL(turnComplete(QColor,int,int,int)),this,SLOT(turnDone(QColor,int,int,int)));
	QPushButton *surrender = widget->findChild<QPushButton*>(QString("btnSurrender"));

	connect(surrender,SIGNAL(clicked()),this,SLOT(playerRetired()));
	playersleft=players.size();
}
*/
Game::Game(QWidget* widget):currplayer(0)
{
	//ui = new Ui::BlokusUi;
	//ui->setupUi(widget);
	ui = dynamic_cast<Ui::MainWindow*>(widget);
	if (ui==0) std::cerr<<"bug!!!!!!" << std::endl;
	table = new Table(20,20);
	QGraphicsScene* tablescene = new QGraphicsScene;
	scenes.append(tablescene);
	tablescene->addItem(table);
	ui->gvTable->setScene(tablescene);
	connect(table,SIGNAL(turnComplete(QColor,int,int,int)),this,SLOT(turnDone(QColor,int,int,int)));
	QPushButton *surrender = widget->findChild<QPushButton*>(QString("btnSurrender"));
	connect(surrender,SIGNAL(clicked()),this,SLOT(playerRetired()));
	this->widget=widget;
	playersleft=0;
}

void Game::addPlayer(QString name,QColor color, PlayerType type)
{
	Player* player;
	switch(type) {
		case(ptLocal):
			player = new LocalPlayer(color);
			break;
		case(ptNetwork):
			player = new NetworkPlayer(color);
			break;
		default:
			return;
			break;
	}
	int i=playersleft;
	QString playerwidget("gvPlayer");
	QString playerscore("score");
	player->setPos(0,0);
	player->setName(name);
	players.append(player);
	QGraphicsScene* playerscene = new QGraphicsScene;
	scenes.append(playerscene);
	playerscene->addItem(player);
	QGraphicsView *gv = widget->findChild<QGraphicsView*>(playerwidget+QString::number(i+1));
	QLCDNumber *lcd = widget->findChild<QLCDNumber*>(playerscore+QString::number(i+1));
	gv->setScene(playerscene);
	
	gv->setMinimumSize(playerscene->sceneRect().size().toSize());
	gv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	gv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	lcd->setPalette(QPalette(color));
	//ui->horizontalLayout->invalidate();
	//ui->gridLayout->invalidate();
	connect(player,SIGNAL(scoreChanged(int)),lcd,SLOT(display(int)));
	connect(player,SIGNAL(iWin(Player* )),this,SLOT(winner(Player *)));
	++playersleft;
}


void Game::turnDone(QColor color, int id,int x,int y)
{
	if (dynamic_cast<Table*>(sender())) {
		emit turnDone(players[currplayer]->getName(),color,id,x,y);
	}
	players[currplayer]->turnComplete(color,id,x,y);
	do {
		currplayer=(currplayer+1)%players.size();
	} while(players[currplayer]->getSurrendered());
	players[currplayer]->startTurn();
}

void Game::playerRetired()
{
	if (dynamic_cast<QPushButton*>(sender())) {
		LocalPlayer* player = dynamic_cast<LocalPlayer*>(players[currplayer]);
		if (!player) return;
		emit playerRetired(players[currplayer]->getName(),players[currplayer]->getColor());
	}
	Player* player = players[currplayer];
	player->surrender();
	--playersleft;
	if (playersleft>0)
	{
		do {
			currplayer=(currplayer+1)%players.size();
		} while(players[currplayer]->getSurrendered());
		if (playersleft==1)
		{
			//winner(players[currplayer]);
			int msp=0;
			for(int p=1;p<players.size();++p) {
				if (players[p]->getScore()>players[msp]->getScore()) msp=p;
				winner(players[msp]);
			}
		} else
		{
			players[currplayer]->startTurn();
		}
	}
	else
	{//Недостижимо!
		//this->deleteLater();
		//emit gameOver();
	}
}

Game::~Game()
{
#ifdef DEBUG_DESTRUCTORS
	std::cerr << "Game over" << std::endl;
#endif
	for (int i=0;i<scenes.size();++i)
	{
		//scenes[i]->deleteLater();
		delete scenes[i];
		delete players[i];
	}
	delete table;
	//delete ui;
}
/*
void Game::clear() {
	for (int i=0;i<scenes.size();++i)
	{
		//scenes[i]->deleteLater();
		delete scenes[i];
		delete players[i];
	}
	playersleft=0;
	currplayer=0;
	scenes.clear();
	players.clear();
//	delete table;
}
*/
void Game::start()
{
	if (players.size()>0)
		players[0]->startTurn();
}

void Game::winner(Player* winner)
{
	std::cerr << winner->getName().toStdString() << "\n";
	emit gameOver(winner->getName(),winner->getScore(),winner->getColor());
}
