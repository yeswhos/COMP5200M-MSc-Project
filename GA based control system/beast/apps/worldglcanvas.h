#ifndef _WORLDGLCANVAS_H
#define _WORLDGLCANVAS_H

#include "wxbeast.h"
#include "wx/glcanvas.h"


#include "world.h"

namespace BEAST {

// Forward declaration for friend class
class BeastFrame;

/**
 * The GL canvas for the main visualisation window.
 */
class WorldGLCanvas : public wxGLCanvas
{
	friend class BeastFrame;

public:
	WorldGLCanvas(World& pWorld, wxWindow *parent, const wxWindowID id = -1,
				 const wxPoint& pos = wxDefaultPosition,
				 const wxSize& size = wxDefaultSize,
		                 long style = wxWANTS_CHARS, const wxString& name = wxString("WorldGLCanvas",*wxConvCurrent));
	~WorldGLCanvas();

	void			OnPaint(wxPaintEvent& event) { wxPaintDC dc(this); Display(); }
	void			Display();
	void			OnSize(wxSizeEvent& event);
	void			OnEraseBackground(wxEraseEvent& event);

	void			OnMouseLeftDown(wxMouseEvent& event);
	void			OnMouseLeftUp(wxMouseEvent& event);
	void			OnMouseRightDown(wxMouseEvent& event);
	void			OnMouseRightUp(wxMouseEvent& event);
	void			OnMouseMove(wxMouseEvent& event);
  void      OnKeyDown(wxKeyEvent& event);
  void      OnKeyUp(wxKeyEvent& event);

	void			InitGL();

	void			Toggle(WorldDisplayType d) { m_pWorld->Toggle(d); }

private:
	World*			m_pWorld;

DECLARE_EVENT_TABLE()
};

} // namespace BEAST

#endif
