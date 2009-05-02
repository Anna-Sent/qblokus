# include <string>
# include <vector>
# include <iostream>
# include <QString>

class Tile
{
	public:
		Tile(std::string type);
		int getHeight() const {return data.size();};
		int getWidth() const {return data.size()>0?data[0].size():0;};
		void rotateTile();
		void reflectTile();
		int score() const;
		std::vector<std::vector<char> > data;
		std::string getAsText() const;
		QString getAsQString() const {return QString(getAsText().c_str());};
		int scor;
};
std::ostream& operator<<(std::ostream&,const Tile&);


