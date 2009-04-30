#include "tile.h"

Tile::Tile(std::string src)
{
	data.resize(1);
	scor=0;
	for(size_t i = 0; i<src.size(); ++i)
	{
		if(src[i]=='|')
		{
			data.resize(data.size()+1);
		}
		else
		{
			data.back().push_back(src[i]);
			if (src[i]!='0') ++scor;
		}
	}
//	rotateTile();
}

void Tile::rotateTile()
{
//	std::cout << "rotate\n";
	std::vector<std::vector<char> > newdata(getWidth(),std::vector<char>(getHeight(),'0'));
	for(int i = 0;i<getHeight();++i)
	{
		for(int j = 0;j<getWidth();++j)
		{
			newdata[j][getHeight()-i-1]=data[i][j];
		}
	}
	data.swap(newdata);
}

int Tile::score() const
{
	return scor;
}

void Tile::reflectTile()
{
//	std::cout << "rotate\n";
	std::vector<std::vector<char> > newdata(getHeight(),std::vector<char>(getWidth(),'0'));
	for(int i = 0;i<getHeight();++i)
	{
		for(int j = 0;j<getWidth();++j)
		{
			newdata[i][getWidth()-j-1]=data[i][j];
		}
	}
	data.swap(newdata);
}

std::string Tile::getAsText() const
{
	std::string result;
	for(int i = 0;i<getHeight();++i)
	{
		for(int j = 0;j<getWidth();++j)
		{
			result.push_back(data[i][j]);
		}
		if (i<getHeight()-1) result.push_back('|');
	}
	return result;
}

std::ostream& operator<<(std::ostream& to,const Tile& what)
{
	for(size_t i=0;i<what.data.size();++i)
	{
		for(size_t j=0;j<what.data[i].size();++j)
		{
			to<<what.data[i][j];
		}
		to<<"\n";
	}
	return to;
}

#ifdef TEST
int main()
{
	Tile tile("12|01");
	std::cout << tile;
}
#endif
