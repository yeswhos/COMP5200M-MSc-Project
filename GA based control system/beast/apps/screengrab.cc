#include "wxbeast.h"
#include "world.h"
#include <wx/image.h>
#include <string>
#include <fstream>

using namespace std;

namespace BEAST {

bool ScreenGrab(World& theWorld, string filename)
{
	int width = theWorld.GetWinWidth();
	int height = theWorld.GetWinHeight();

	glDrawBuffer(GL_AUX0);
	theWorld.Display();
	glReadBuffer(GL_AUX0);

	GLubyte* pixelData = new GLubyte[width * height * 3];

	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixelData);

	wxImage image(width, height, reinterpret_cast<unsigned char*>(pixelData), true);
	wxImage mirrored(image.Mirror(false));

	wxImage::AddHandler(new wxJPEGHandler);

	bool result = mirrored.SaveFile(wxString(filename.c_str(),*wxConvCurrent), wxBITMAP_TYPE_JPEG);

	delete[] pixelData;

	return result;
}

} // namespace BEAST
