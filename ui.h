#ifndef FIGRID_UI_H
#define FIGRID_UI_H

#include "figrid.h"

namespace Namespace_Figrid
{

class Figrid_UI
{
protected:
	Figrid* figrid;
	Figrid_UI(Figrid* f): figrid(f) {}

public:	
	virtual int run() = 0;
	virtual void refresh() = 0;
};

}
#endif

