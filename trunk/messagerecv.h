#ifndef MESSAGERECV_H_
#define MESSAGERECV_H_
#include <QObject>
#include <QList>
#include <QColor>
#include <QByteArray>
#include <QString>
#include <QMetaType>
#include "clientinfo.h"

enum MessageType { mtHeader, mtChat, mtPlayersList,
	mtClientConnect, mtClientDisconnect, mtServerReady,
	mtConnectionAccepted, mtPing, mtTryToConnect,
	mtStartGame, mtRestartGame, mtTurn, mtSurrender };

class TCPSocket;
class UDPSocket;

class Message {
	protected:
		MessageType code;
	public:
		virtual qint64 getLength() const = 0;
		virtual void fill(const QByteArray&) = 0;
		virtual Message* next() const;
		virtual QByteArray serialize() const = 0;
		void send(TCPSocket*) const;
};

class MessageHeader : public Message {
	public:
		MessageType type;
		qint64 len;
		qint64 getLength() const { return sizeof(MessageType) + sizeof(qint64); }
		Message* next() const;
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class ComplexMessage : public Message {
	protected:
		MessageHeader header;
	public:
		qint64 getLength() const {return header.len;}
		QByteArray serialize() const { return header.serialize(); }
		void fill(const QByteArray&) {}
};

class ChatMessage : public ComplexMessage {
	private:
		QString text;
		ClientInfo info;
	public:
		ChatMessage() {}
		ChatMessage(const MessageHeader& header);
		QString getName() const {return info.name;}
		QString getText() const {return text;}
		QColor getColor() const {return info.color;}
		ChatMessage(QString name, QString text, QColor color);
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class PlayersListMessage : public ComplexMessage {
	protected:
		QList<ClientInfo> list;
	public:
		PlayersListMessage() {}
		PlayersListMessage(const MessageHeader& header);
		QList<ClientInfo> getList() const {return list;}
		PlayersListMessage(QList<ClientInfo>);
		QByteArray serialize() const;
		void fill(const QByteArray&);
};

class RestartGameMessage : public PlayersListMessage {
	public:
		RestartGameMessage() {}
		RestartGameMessage(const MessageHeader& header);
		RestartGameMessage(QList<ClientInfo>);
};

class ClientMessage : public ComplexMessage {
	protected:
		ClientInfo info;
	public:
		QString getName() const {return info.name;}
		QColor getColor() const {return info.color;}
		ClientInfo getInfo() const {return info;}
		virtual QByteArray serialize() const;
		virtual void fill(const QByteArray&);
};

class TryToConnectMessage : public ClientMessage {
	public:
		TryToConnectMessage(const MessageHeader& header) {this->header=header;}
		TryToConnectMessage(ClientInfo);
};

class ClientConnectMessage : public ClientMessage {
	public:
		ClientConnectMessage() {}
		ClientConnectMessage(const MessageHeader& header) {this->header=header;}
		ClientConnectMessage(QString, QColor);
};

class ClientDisconnectMessage : public ClientMessage {
	public:
		ClientDisconnectMessage() {}
		ClientDisconnectMessage(const MessageHeader& header) {this->header=header;}
		ClientDisconnectMessage(QString, QColor);
};

class ServerReadyMessage : public ComplexMessage {
	private:
	public:
		ServerReadyMessage(const MessageHeader& header) {this->header=header;}
		ServerReadyMessage() { header.len = 0; header.type = mtServerReady;}
};


class PingMessage : public ComplexMessage {
	private:
	public:
		PingMessage(const MessageHeader& header) {this->header=header;}
		PingMessage() { header.len = 0; header.type = mtPing;}
};

class ConnectionAcceptedMessage : public ComplexMessage {
	private:
		int errorCode;
	public:
		ConnectionAcceptedMessage() {};
		ConnectionAcceptedMessage(const MessageHeader& header) {this->header=header;}
		ConnectionAcceptedMessage(int errorCode);
		QByteArray serialize() const;
		void fill(const QByteArray&);
		int getCode() const {return errorCode;}
};

class StartGameMessage : public ComplexMessage {
	private:
	public:
		StartGameMessage(const MessageHeader& header) {this->header=header;}
		StartGameMessage() { header.len = 0; header.type = mtStartGame;}
};

class TurnMessage : public ClientMessage {
private:
	int id, x, y;
	QString mask;
public:
	TurnMessage() {}
	TurnMessage(const MessageHeader& header) {this->header=header;}
	TurnMessage(QString, QColor, QString,int id, int x, int y);
	int getX() const {return x;}
	int getY() const {return y;}
	int getId() const {return id;}
	QString getMask() const {return mask;}
	QByteArray serialize() const;
	void fill(const QByteArray&);
};

class SurrenderMessage : public ClientMessage {
	public:
		SurrenderMessage() {}
		SurrenderMessage(QString name, QColor color)
		{
			info.name=name;
			info.color=color;
			header.type=mtSurrender;
			header.len=info.size();
		}
		SurrenderMessage(const MessageHeader& header)
		{
			this->header=header;
		}
};

class MessageReceiver : public QObject {
	Q_OBJECT
	private:
		TCPSocket* socket;
		Message* current;
		QByteArray buffer;
	public:
		MessageReceiver(TCPSocket*);
		~MessageReceiver();
		TCPSocket* getSocket() {return socket;}
	public slots:
		void readyRead();
	signals:
		void chatMessageReceive(ChatMessage);
		void playersListMessageReceive(PlayersListMessage);
		void restartGameMessageReceive(RestartGameMessage);
		void tryToConnectMessageReceive(TryToConnectMessage);
		void clientConnectMessageReceive(ClientConnectMessage);
		void clientDisconnectMessageReceive(ClientDisconnectMessage);
		void getMessage(QByteArray);
		void serverReadyMessageReceive(ServerReadyMessage);
		void connectionAcceptedMessageReceive(ConnectionAcceptedMessage);
		void pingMessageReceive(PingMessage);
		void startGameMessageReceive(StartGameMessage);
		void turnMessageReceive(TurnMessage);
		void surrenderMessageReceive(SurrenderMessage);
};
#endif