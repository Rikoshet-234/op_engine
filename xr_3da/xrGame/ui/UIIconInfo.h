#ifndef UIIconInfo_H
#define UIIconInfo_H

class UIIconInfo
{
private:
	Ivector4 iconCoords;
public:
	UIIconInfo() { iconCoords.set(0,0,0,0);}
	UIIconInfo(const shared_str itemSection) {Load(itemSection,true);}
	void Load(const shared_str itemSection,bool raise=true);

	int getX() const {return iconCoords.x;}
	int getY() const {return iconCoords.y;}
	int getWidth() const {return iconCoords.z;}
	int getHeight() const {return iconCoords.w;}

	Frect& getOriginalRect() const;

	void setCoords(int x ,int y,int width,int height) {iconCoords.set(x,y,width,height);}
	Ivector4 getCoords() const	{return iconCoords;}
};
#endif