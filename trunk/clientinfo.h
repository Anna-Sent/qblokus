#include <QString>
#include <QColor>
#include <QByteArray>

class ClientInfo {
	public:
		QString name;
		QColor color;
		int state;
		QByteArray serialize() const;
		void fill(const char*);
		int size() const;
		ClientInfo():state(0),name(""),color(Qt::black){}
};