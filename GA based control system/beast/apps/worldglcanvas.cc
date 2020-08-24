#include "worldglcanvas.h"
#include <cstdlib>

using namespace std;

namespace BEAST {

BEGIN_EVENT_TABLE(WorldGLCanvas, wxGLCanvas)
  EVT_SIZE				(				WorldGLCanvas::OnSize)
  EVT_PAINT				(				WorldGLCanvas::OnPaint)
  EVT_ERASE_BACKGROUND	(				WorldGLCanvas::OnEraseBackground)

  EVT_LEFT_DOWN			(				WorldGLCanvas::OnMouseLeftDown)
  EVT_LEFT_UP			(				WorldGLCanvas::OnMouseLeftUp)
  EVT_RIGHT_DOWN		(				WorldGLCanvas::OnMouseRightDown)
  EVT_RIGHT_UP			(				WorldGLCanvas::OnMouseRightUp)
  EVT_MOTION			(				WorldGLCanvas::OnMouseMove)
  EVT_KEY_DOWN (WorldGLCanvas::OnKeyDown)
  EVT_KEY_UP (WorldGLCanvas::OnKeyUp)
END_EVENT_TABLE()

WorldGLCanvas::WorldGLCanvas(World& rWorld, wxWindow *parent, wxWindowID id,
						   const wxPoint& pos, const wxSize& size,
						   long style, const wxString& name):
wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style, name ),
m_pWorld(&rWorld)
{
	SetCurrent();
	InitGL();

  m_pWorld->SetWXKLeft(WXK_LEFT);
  m_pWorld->SetWXKRight(WXK_RIGHT);
  m_pWorld->SetWXKUp(WXK_UP);
  m_pWorld->SetWXKDown(WXK_DOWN);
  
  SetFocus();
}

WorldGLCanvas::~WorldGLCanvas()
{
}

void WorldGLCanvas::Display()
{
	if (!GetContext()) return;

	SetCurrent();

	m_pWorld->Display();

	glFlush();
	SwapBuffers();
}

void WorldGLCanvas::OnSize(wxSizeEvent& event)
{
    int width, height;
    GetClientSize(&width, &height);

	if (GetContext())
    {
        SetCurrent();
        glViewport(0, 0, event.GetSize().GetWidth(), event.GetSize().GetHeight());
    }

	m_pWorld->SetWindow(event.GetSize().GetWidth(), event.GetSize().GetHeight());
}

void WorldGLCanvas::OnEraseBackground(wxEraseEvent& event)
{
  // Do nothing, to avoid flashing.
}

void WorldGLCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
	m_pWorld->OnMouseLDown(event.GetX(), event.GetY());
}

void WorldGLCanvas::OnMouseLeftUp(wxMouseEvent& event)
{
	m_pWorld->OnMouseLUp(event.GetX(), event.GetY());
}

void WorldGLCanvas::OnMouseRightDown(wxMouseEvent& event)
{
	m_pWorld->OnMouseRDown(event.GetX(), event.GetY());
}

void WorldGLCanvas::OnMouseRightUp(wxMouseEvent& event)
{
	m_pWorld->OnMouseRUp(event.GetX(), event.GetY());
}

void WorldGLCanvas::OnMouseMove(wxMouseEvent& event)
{
	m_pWorld->OnMouseMove(event.GetX(), event.GetY());
}

void WorldGLCanvas::OnKeyDown(wxKeyEvent& event)
{
  m_pWorld->OnKeyDown(event.GetKeyCode(), event.GetUnicodeKey(), event.ShiftDown());
}

void WorldGLCanvas::OnKeyUp(wxKeyEvent& event)
{
  m_pWorld->OnKeyUp(event.GetKeyCode(), event.GetUnicodeKey(), event.ShiftDown());
}

void WorldGLCanvas::InitGL()
{
    SetCurrent();

	m_pWorld->InitGL();
}

} // namespace BEAST
