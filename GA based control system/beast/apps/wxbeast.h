#ifndef _BEASTGLOBAL_H
#define _BEASTGLOBAL_H

#ifdef This
#undef This
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef new
#undef new
#endif

#include "beast.h"

#ifdef This
#undef This
#endif

namespace BEAST {

class BeastFrame;

/**
 * The main Beast application class.
 */
class BeastApp : public wxApp
{
public:
    bool			OnInit();

private:
	BeastFrame*	m_pBeastFrame;
};

} // namespace BEAST

#endif
