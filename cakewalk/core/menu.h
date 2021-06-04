// used: std::function
#include <functional>

// used: winapi, directx, imgui, fmt includes
#include "../common.h"

/*
 * WINDOWS
 */
namespace W
{
	// Get
	/* main cheat window */
	void MainWindow(IDirect3DDevice9* pDevice);

	// Values
	// @note: add smth like Vars.iMainExtra to show extra functions in menu e.g. hitchance precision and other (if disabled, will be used default optimal values)
	/* open state of main window */
	inline bool bMainOpened = false;
}

namespace T
{
	void RenderWalkBot();
	void RenderAimBot();
	void RenderMisc();
	void RenderConfig();
}