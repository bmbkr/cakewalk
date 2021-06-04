#include "draw.h"

// used: engine interface
#include "../core/interfaces.h"
// used: current window
#include "inputsystem.h"


#pragma region draw_get
void D::Setup(IDirect3DDevice9* pDevice, unsigned int uFontFlags)
{
	ImGui::CreateContext();

	// setup platform and renderer bindings
	ImGui_ImplWin32_Init(IPT::hWindow);
	ImGui_ImplDX9_Init(pDevice);

	// create fonts
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig imVerdanaConfig;
	imVerdanaConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
	F::Verdana = io.Fonts->AddFontFromFileTTF(XorStr("C:\\Windows\\Fonts\\Verdana.ttf"), 14.f, &imVerdanaConfig, io.Fonts->GetGlyphRangesCyrillic());

	bInitialized = ImGuiFreeType::BuildFontAtlas(io.Fonts, uFontFlags);
}

void D::Destroy()
{
	// shutdown imgui directx9 renderer binding
	ImGui_ImplDX9_Shutdown();

	// shutdown imgui win32 platform binding
	ImGui_ImplWin32_Shutdown();

	// destroy imgui context
	ImGui::DestroyContext();
}

void D::RenderDrawData(ImDrawList* pDrawList)
{
	std::unique_lock<std::shared_mutex> lock(drawMutex);

	// prevent render in main menu
	if (!I::Engine->IsInGame())
		return;

	if (vecSafeDrawData.empty())
		return;

	for (const auto& data : vecSafeDrawData)
	{
		if (!data.pObject.has_value())
			continue;

		switch (data.nType)
		{
		case EDrawType::LINE:
		{
			const auto& pObject = std::any_cast<LineObject_t>(data.pObject);
			pDrawList->AddLine(pObject.vecStart, pObject.vecEnd, pObject.colLine, pObject.flThickness);
			break;
		}
		case EDrawType::RECT:
		{
			const auto& pObject = std::any_cast<RectObject_t>(data.pObject);
			AddDrawListRect(pDrawList, pObject.vecMin, pObject.vecMax, pObject.colRect, pObject.uFlags, pObject.colOutline, pObject.flRounding, pObject.roundingCorners, pObject.flThickness);
			break;
		}
		case EDrawType::RECT_MULTICOLOR:
		{
			const auto& pObject = std::any_cast<RectMultiColorObject_t>(data.pObject);
			pDrawList->AddRectFilledMultiColor(pObject.vecMin, pObject.vecMax, pObject.colUpperLeft, pObject.colUpperRight, pObject.colBottomRight, pObject.colBottomLeft);
			break;
		}
		case EDrawType::CIRCLE:
		{
			const auto& pObject = std::any_cast<CircleObject_t>(data.pObject);

			if (pObject.uFlags & DRAW_CIRCLE_FILLED)
				pDrawList->AddCircleFilled(pObject.vecCenter, pObject.flRadius, pObject.colCircle, pObject.nSegments);
			else
				pDrawList->AddCircle(pObject.vecCenter, pObject.flRadius, pObject.colCircle, pObject.nSegments, pObject.flThickness);

			if (pObject.uFlags & DRAW_CIRCLE_OUTLINE)
				pDrawList->AddCircle(pObject.vecCenter, pObject.flRadius + 1.0f, pObject.colOutline, pObject.nSegments, pObject.flThickness + 1.0f);

			break;
		}
		case EDrawType::TRIANGLE:
		{
			const auto& pObject = std::any_cast<TriangleObject_t>(data.pObject);

			if (pObject.uFlags & DRAW_TRIANGLE_FILLED)
				pDrawList->AddTriangleFilled(pObject.vecFirst, pObject.vecSecond, pObject.vecThird, pObject.colTriangle);
			else
				pDrawList->AddTriangle(pObject.vecFirst, pObject.vecSecond, pObject.vecThird, pObject.colTriangle, pObject.flThickness);

			if (pObject.uFlags & DRAW_TRIANGLE_OUTLINE)
				pDrawList->AddTriangle(pObject.vecFirst, pObject.vecSecond, pObject.vecThird, pObject.colOutline, pObject.flThickness + 1.0f);

			break;
		}
		case EDrawType::POLYGON:
		{
			const auto& pObject = std::any_cast<PolygonObject_t>(data.pObject);

			if (pObject.uFlags & DRAW_POLYGON_FILLED)
				pDrawList->AddConvexPolyFilled(pObject.vecPoints.data(), pObject.vecPoints.size(), pObject.colPolygon);
			else
				pDrawList->AddPolyline(pObject.vecPoints.data(), pObject.vecPoints.size(), pObject.colPolygon, pObject.bClosed, pObject.flThickness);

			if (pObject.uFlags & DRAW_POLYGON_OUTLINE)
				pDrawList->AddPolyline(pObject.vecPoints.data(), pObject.vecPoints.size(), pObject.colOutline, pObject.bClosed, pObject.flThickness + 1.0f);

			break;
		}
		case EDrawType::TEXT:
		{
			const auto& pObject = std::any_cast<TextObject_t>(data.pObject);
			AddDrawListText(pDrawList, pObject.pFont, pObject.flFontSize, pObject.vecPosition, pObject.szText, pObject.colText, pObject.uFlags, pObject.colOutline);
			break;
		}
		case EDrawType::IMAGE:
		{
			const auto& pObject = std::any_cast<ImageObject_t>(data.pObject);
			pDrawList->AddImageRounded(pObject.pTexture, pObject.vecMin, pObject.vecMax, ImVec2(0, 0), ImVec2(1, 1), pObject.colImage, pObject.flRounding, pObject.roundingCorners);
			break;
		}
		default:
			break;
		}
	}
}

void D::ClearDrawData()
{
	if (!vecDrawData.empty())
		vecDrawData.clear();
}

void D::SwapDrawData()
{
	std::unique_lock<std::shared_mutex> lock(drawMutex);
	vecDrawData.swap(vecSafeDrawData);
}
#pragma endregion

#pragma region draw_render
void D::AddLine(const ImVec2& vecStart, const ImVec2& vecEnd, const Color& colLine, float flThickness)
{
	vecDrawData.emplace_back(EDrawType::LINE, std::make_any<LineObject_t>(LineObject_t{ vecStart, vecEnd, colLine.GetU32(), flThickness }));
}

void D::AddRect(const ImVec2& vecMin, const ImVec2& vecMax, const Color& colRect, unsigned int uFlags, const Color& colOutline, float flRounding, ImDrawCornerFlags roundingCorners, float flThickness)
{
	vecDrawData.emplace_back(EDrawType::RECT, std::make_any<RectObject_t>(RectObject_t{ vecMin, vecMax, colRect.GetU32(), uFlags, colOutline.GetU32(), flRounding, roundingCorners, flThickness }));
}

void D::AddRectMultiColor(const ImVec2& vecMin, const ImVec2& vecMax, const Color& colUpperLeft, const Color& colUpperRight, const Color& colBottomRight, const Color& colBottomLeft)
{
	vecDrawData.emplace_back(EDrawType::RECT_MULTICOLOR, std::make_any<RectMultiColorObject_t>(RectMultiColorObject_t{ vecMin, vecMax, colUpperLeft.GetU32(), colUpperRight.GetU32(), colBottomRight.GetU32(), colBottomLeft.GetU32() }));
}

void D::AddCircle(const ImVec2& vecCenter, float flRadius, const Color& colCircle, int nSegments, unsigned int uFlags, const Color& colOutline, float flThickness)
{
	vecDrawData.emplace_back(EDrawType::CIRCLE, std::make_any<CircleObject_t>(CircleObject_t{ vecCenter, flRadius, colCircle.GetU32(), nSegments, uFlags, colOutline.GetU32(), flThickness }));
}

void D::AddTriangle(const ImVec2& vecFirst, const ImVec2& vecSecond, const ImVec2& vecThird, const Color& colTriangle, unsigned int uFlags, const Color& colOutline, float flThickness)
{
	vecDrawData.emplace_back(EDrawType::TRIANGLE, std::make_any<TriangleObject_t>(TriangleObject_t{ vecFirst, vecSecond, vecThird, colTriangle.GetU32(), uFlags, colOutline.GetU32(), flThickness }));
}

void D::AddPolygon(std::vector<ImVec2>& vecPoints, const Color& colPolygon, unsigned int uFlags, const Color& colOutline, bool bClosed, float flThickness)
{
	vecDrawData.emplace_back(EDrawType::POLYGON, std::make_any<PolygonObject_t>(PolygonObject_t{ std::move(vecPoints), colPolygon.GetU32(), uFlags, colOutline.GetU32(), bClosed, flThickness }));
}

void D::AddText(const ImFont* pFont, float flFontSize, const ImVec2& vecPosition, const std::string& szText, const Color& colText, unsigned int uFlags, const Color& colOutline)
{
	if (pFont->ContainerAtlas == nullptr)
		return;

	// check is only one flag selected
	IM_ASSERT(ImIsPowerOfTwo(uFlags == DRAW_TEXT_NONE || uFlags & (DRAW_TEXT_DROPSHADOW | DRAW_TEXT_OUTLINE)));

	vecDrawData.emplace_back(EDrawType::TEXT, std::make_any<TextObject_t>(TextObject_t{ pFont, flFontSize, vecPosition, szText, colText.GetU32(), uFlags, colOutline.GetU32() }));
}

void D::AddText(const ImVec2& vecPosition, const std::string& szText, const Color& colText, int iFlags, const Color& colOutline)
{
	AddText(nullptr, 0.f, vecPosition, szText, colText, iFlags, colOutline);
}

void D::AddImage(ImTextureID pTexture, const ImVec2& vecMin, const ImVec2& vecMax, const Color& colImage, float flRounding, ImDrawCornerFlags roundingCorners)
{
	vecDrawData.emplace_back(EDrawType::IMAGE, std::make_any<ImageObject_t>(ImageObject_t{ pTexture, vecMin, vecMax, colImage.GetU32(), flRounding, roundingCorners }));
}
#pragma endregion

#pragma region draw_bindings
void D::AddDrawListRect(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, ImU32 colRect, unsigned int uFlags, ImU32 colOutline, float flRounding, ImDrawCornerFlags roundingCorners, float flThickness)
{
	if (uFlags & DRAW_RECT_FILLED)
		pDrawList->AddRectFilled(vecMin, vecMax, colRect, flRounding, roundingCorners);
	else
		pDrawList->AddRect(vecMin, vecMax, colRect, flRounding, roundingCorners, flThickness);

	if (uFlags & DRAW_RECT_BORDER)
		pDrawList->AddRect(vecMin + ImVec2(1.0f, 1.0f), vecMax - ImVec2(1.0f, 1.0f), colOutline, flRounding, roundingCorners, 1.0f);

	if (uFlags & DRAW_RECT_OUTLINE)
		pDrawList->AddRect(vecMin - ImVec2(1.0f, 1.0f), vecMax + ImVec2(1.0f, 1.0f), colOutline, flRounding, roundingCorners, 1.0f);
}

void D::AddDrawListText(ImDrawList* pDrawList, const ImFont* pFont, float flFontSize, const ImVec2& vecPosition, const std::string& szText, ImU32 colText, unsigned int uFlags, ImU32 colOutline)
{
	// set font texture
	pDrawList->PushTextureID(pFont->ContainerAtlas->TexID);

	ImVec2(2, 2) + ImVec2(2, 2);

	if (uFlags & DRAW_TEXT_DROPSHADOW)
		pDrawList->AddText(vecPosition + ImVec2(1.0f, -1.0f), colOutline, szText.c_str());
	else if (uFlags & DRAW_TEXT_OUTLINE)
	{
		pDrawList->AddText(vecPosition + ImVec2(1.0f, -1.0f), colOutline, szText.c_str());
		pDrawList->AddText(vecPosition + ImVec2(-1.0f, 1.0f), colOutline, szText.c_str());
	}

	pDrawList->AddText(vecPosition, colText, szText.data());
	pDrawList->PopTextureID();
}
#pragma endregion

#pragma region draw_extra
bool D::WorldToScreen(const Vector& vecOrigin, ImVec2& vecScreen)
{
	const ViewMatrix_t& matWorldToScreen = I::Engine->WorldToScreenMatrix();
	const float flWidth = matWorldToScreen[3][0] * vecOrigin.x + matWorldToScreen[3][1] * vecOrigin.y + matWorldToScreen[3][2] * vecOrigin.z + matWorldToScreen[3][3];

	if (flWidth < 0.001f)
		return false;

	// compute the scene coordinates of a point in 3d
	const float flInverse = 1.0f / flWidth;
	vecScreen.x = (matWorldToScreen[0][0] * vecOrigin.x + matWorldToScreen[0][1] * vecOrigin.y + matWorldToScreen[0][2] * vecOrigin.z + matWorldToScreen[0][3]) * flInverse;
	vecScreen.y = (matWorldToScreen[1][0] * vecOrigin.x + matWorldToScreen[1][1] * vecOrigin.y + matWorldToScreen[1][2] * vecOrigin.z + matWorldToScreen[1][3]) * flInverse;

	// screen transform
	// get the screen position in pixels of given point
	const ImVec2 vecDisplaySize = ImGui::GetIO().DisplaySize;
	vecScreen.x = (vecDisplaySize.x * 0.5f) + (vecScreen.x * vecDisplaySize.x) * 0.5f;
	vecScreen.y = (vecDisplaySize.y * 0.5f) - (vecScreen.y * vecDisplaySize.y) * 0.5f;
	return true;
}
#pragma endregion
