#include "game.h"
#include <QColor>
#include <QGraphicsScene>
#include "table.h"
#include "localplayer.h"
#include "networkplayer.h"
#include "ui_racingForm.h"
#include <iostream>
#include <QColor>
#include "clientinfo.h"

Game::Game(QWidget* widget):currplayer(0),running(false)
{
	//ui = new Ui::BlokusUi;
	//ui->setupUi(widget);
	ui = dynamic_cast<Ui::MainWindow*>(widget);
	if (ui==0) std::cerr<<"bug!!!!!!" << std::endl;
	table = new Table(20,20);
	tablescene = new QGraphicsScene;
	//scenes.append(tablescene);
	tablescene->addItem(table);
	ui->gvTable->setScene(tablescene);
	connect(table,SIGNAL(turnComplete(QColor,QString,int,int,int)),this,SLOT(turnDone(QColor,QString,int,int,int)));
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
			player = new NetworkPlayer(color,table);
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


void Game::turnDone(QColor color, QString tile,int id,int x,int y)
{
	if (color!=players[currplayer]->getColor()) return;
	if (dynamic_cast<Table*>(sender())) {
		std::cerr << "111\n";
		emit turnDone(players[currplayer]->getName(),color,tile,id,x,y);
	}
	players[currplayer]->turnComplete(color,tile,id,x,y);
	do {
		currplayer=(currplayer+1)%players.size();
	} while(players[currplayer]->getSurrendered());
	players[currplayer]->startTurn();
	std::cerr << "===== " << currplayer << " ====\n";
}

void Game::playerRetired()
{
	if (sender()&&dynamic_cast<QPushButton*>(sender())) {
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
			for(int p=1;p<players.size();++p)
				if (players[p]->getScore()>players[msp]->getScore()) msp=p;
			winner(players[msp]);
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
/*	for (int i=0;i<scenes.size();++i)
	{
		//scenes[i]->deleteLater();
		delete scenes[i];
		//delete players[i];
	}
	scenes.clear();*/
	clear();
	delete tablescene;
	//delete table;
	//delete ui;
}

void Game::clear() {
	for (int i=0;i<scenes.size();++i)
	{
		//scenes[i]->deleteLater();
		delete scenes[i];
//		scenes.removeAt(1);
		//delete players[i];
	}
	playersleft=0;
	currplayer=0;
	scenes.clear();
	players.clear();
	//QList<QLCDNumber*> *lcd = widget->findChild<QLCDNumber*>(playerscore+QString::number(i+1));
	QList<QLCDNumber*> lcds = widget->findChildren<QLCDNumber*>();
	for(int i=0;i<lcds.size();++i)
	{
		lcds[i]->setPalette(QPalette());
	}
//	delete table;
}

void Game::start()
{
	if (players.size()>0)
		players[0]->startTurn();
	running=true;
}

void Game::winner(Player* winner)
{
	std::cerr << winner->getName().toStdString() << "\n";
	emit gameOver(winner->getName(),winner->getScore(),winner->getColor());
}

bool operator==(const ClientInfo& a1,const ClientInfo& a2)
{
	return a1.name==a2.name&&a1.color==a2.color;
}

void Game::retirePlayer(int i)
{
	std::cerr << i << " ======= " << currplayer << std::endl; 
	if (currplayer==i) 
	{
		playerRetired();
		return;
	}
	Player *player=players[i];
	if (!player->surrendered)
	{
		player->surrendered=true;
		--playersleft;
	}
	if (playersleft==1)
	{
		//winner(players[currplayer]);
		int msp=0;
		for(int p=1;p<players.size();++p)
			if (players[p]->getScore()>players[msp]->getScore()) msp=p;
		winner(players[msp]);
	}
	player->update();

}

void Game::updatePlayers(QList<ClientInfo> clients,QList<bool> local)
{
	std::cerr << players.size() << " " << clients.size() << " " << local.size() << std::endl;
	if (!running)
	{
		int i=0;
		clear();
		for(int i=0;i<clients.size();++i)
		{
			addPlayer(clients[i].name,clients[i].color,local[i]?ptLocal:ptNetwork);
		}		
	}
	else
	{
		//merge
		int pl=0,cl=0;
		while(pl<players.size()||cl<clients.size())
		{
			if (pl==players.size()||cl==clients.size())
			{
				if (pl==players.size())
				{
					addPlayer(clients[cl].name,clients[cl].color,local[cl]?ptLocal:ptNetwork);
				}
				if (cl==clients.size())
				{
					for(int i=pl;i<players.size();++i)
					{
						retirePlayer(i);
					}
					pl=players.size();
				}
			} else {
				std::cerr << players[pl]->getName().toStdString() << " " << clients[cl].name.toStdString() << std::endl;
				if (players[pl]->getName()==clients[cl].name&&players[pl]->getColor()==clients[cl].color)
				{
					++pl;
					++cl;
				}
				else
				{
					retirePlayer(pl);
					++pl;			
				}
			}
		}
	}
	std::cerr << "----------players updated\n";
}
bool Game::isStarted() const
{
	return running;
}
