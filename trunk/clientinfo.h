#include <QString>
#include <QColor>
#include <QByteArray>

class ClientInfo {
	public:
		QString name;
		QColor color;
		QByteArray serialize() const;
		void fill(const char*);
		int size() const;
};