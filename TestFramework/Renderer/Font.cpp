// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Font.h>
#include <Renderer/Renderer.h>
#include <Image/Surface.h>
#include <Jolt/Core/Profiler.h>

const int cSpacingH		= 2;								// Number of pixels to put horizontally between characters
const int cSpacingV		= 2;								// Number of pixels to put vertically between characters

Font::Font(Renderer *inRenderer) :
	mRenderer(inRenderer)
{
}

bool
Font::Create(const char *inFontName, int inCharHeight)
{
	JPH_PROFILE("Create");

	// Initialize
	mFontName = inFontName;
	mCharHeight = inCharHeight;
	mHorizontalTexels = 64;
	mVerticalTexels = 64;

	// Check font name length
	if (mFontName.size() >= LF_FACESIZE)
		return false;

	// Create font
	LOGFONTA font_desc;
	memset(&font_desc, 0, sizeof(font_desc));
	font_desc.lfHeight			= mCharHeight;
	font_desc.lfWeight			= FW_NORMAL;
	font_desc.lfCharSet			= DEFAULT_CHARSET;
	font_desc.lfOutPrecision	= OUT_DEFAULT_PRECIS;
	font_desc.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	font_desc.lfQuality			= ANTIALIASED_QUALITY;
	font_desc.lfPitchAndFamily	= VARIABLE_PITCH;
	strcpy_s(font_desc.lfFaceName, mFontName.c_str());
	HFONT font = CreateFontIndirectA(&font_desc);
    if (font == nullptr)
		return false;

	// Create a DC for the font
    HDC dc = CreateCompatibleDC(nullptr);
	if (dc == nullptr)
	{
		DeleteObject(font);
		return false;
	}

	// Select the font
	SelectObject(dc, font);
    SetMapMode(dc, MM_TEXT);

	// Get text metrics
	TEXTMETRICA textmetric;
	if (!GetTextMetricsA(dc, &textmetric))
	{
		DeleteObject(font);
		DeleteDC(dc);
		return false;
	}

	// Compute spacing
	ABC widths[256];
	if (!GetCharABCWidthsA(dc, 0, 255, widths))
	{
		DeleteObject(font);
		DeleteDC(dc);
		return false;
	}
	for (int idx1 = 0; idx1 < cNumChars; ++idx1)
		for (int idx2 = 0; idx2 < cNumChars; ++idx2)
		{
			int spacing = int(widths[idx1 + cBeginChar].abcB) + widths[idx1 + cBeginChar].abcC + widths[idx2 + cBeginChar].abcA;
			JPH_ASSERT(spacing >= 0 && spacing <= 0xff);
			mSpacing[idx1][idx2] = (uint8)spacing;
		}

	// Adjust spacing for kerning pairs
	DWORD pair_count = GetKerningPairsA(dc, 0, nullptr);
	if (pair_count > 0)
	{
		LPKERNINGPAIR pairs = new KERNINGPAIR [pair_count];
		GetKerningPairsA(dc, pair_count, pairs);
		for (DWORD i = 0; i < pair_count; ++i)
			if (pairs[i].wFirst >= cBeginChar && pairs[i].wFirst < cEndChar && pairs[i].wSecond >= cBeginChar && pairs[i].wSecond < cEndChar)
			{
				int idx1 = pairs[i].wFirst - cBeginChar;
				int idx2 = pairs[i].wSecond - cBeginChar;

				int new_spacing = (int)mSpacing[idx1][idx2] + pairs[i].iKernAmount;
				JPH_ASSERT(new_spacing >= 0 && new_spacing <= 0xff);
				mSpacing[idx1][idx2] = (uint8)new_spacing;
			}
		delete [] pairs;
	}

	// Create surface for characters
	Ref<SoftwareSurface> surface = new SoftwareSurface(mHorizontalTexels, mVerticalTexels, ESurfaceFormat::L8);
	surface->Clear();
	surface->Lock(ESurfaceLockMode::Write);

	// Identity transform
	MAT2 identity;
	memset(&identity, 0, sizeof(identity));
	identity.eM11.value = 1;
	identity.eM22.value = 1;

	// Draw all printable characters
	try_again:;
	int x = 0, y = 0;
	static_assert(cBeginChar == ' ', "We skip space in the for loop below");
	for (int c = cBeginChar + 1; c < cEndChar; ++c)
	{
		// Get index in the arrays
		int idx = c - cBeginChar;

		// Check if there is room on this line
		if (int(x + widths[c].abcB + cSpacingH) > mHorizontalTexels)
		{
			// Next line
			x = 0;
			y += textmetric.tmHeight + cSpacingV;

			// Check if character fits
			if (y + textmetric.tmHeight + cSpacingV > mVerticalTexels)
			{
				// Character doesn't fit, enlarge surface
				if (mHorizontalTexels < 2 * mVerticalTexels)
					mHorizontalTexels <<= 1;
				else
					mVerticalTexels <<= 1;

				// Create new surface
				surface->UnLock();
				surface = new SoftwareSurface(mHorizontalTexels, mVerticalTexels, ESurfaceFormat::L8);
				surface->Clear();
				surface->Lock(ESurfaceLockMode::Write);

				// Try again with the larger texture
				goto try_again;
			}
		}

		// Get location of character in font surface
		JPH_ASSERT(x >= 0 && x <= 0xffff);
		JPH_ASSERT(y >= 0 && y <= 0xffff);
		JPH_ASSERT(widths[c].abcB <= 0xff);
		mStartU[idx] = uint16(x);
		mStartV[idx] = uint16(y);
		mWidth[idx] = uint8(widths[c].abcB);

		// Get character data size
		GLYPHMETRICS metrics;
		int char_size = GetGlyphOutlineA(dc, c, GGO_GRAY8_BITMAP, &metrics, 0, nullptr, &identity);
		if (char_size != 0)
		{
			// Allocate room for character
			uint8 *char_data = new uint8 [char_size];

			// Get character
			GetGlyphOutlineA(dc, c, GGO_GRAY8_BITMAP, &metrics, char_size, char_data, &identity);
			uint src_pitch = (metrics.gmBlackBoxX + 3) & ~uint(3);

			// Copy the character data
			for (uint src_y = 0, dst_y = y; src_y < metrics.gmBlackBoxY; ++src_y, ++dst_y)
			{
				uint8 *src = char_data + src_y * src_pitch;
				uint8 *dst = surface->GetScanLine(dst_y + int(textmetric.tmHeight - textmetric.tmDescent - metrics.gmptGlyphOrigin.y)) + x;

				for (uint src_x = 0; src_x < metrics.gmBlackBoxX; ++src_x, ++src, ++dst)
					*dst = uint8(min(int(*src) << 2, 255));
			}

			// Destroy temporary character data
			delete [] char_data;
		}

		// Go to the next character
		x += widths[c].abcB + cSpacingH;
	}

	// Unlock surface
	surface->UnLock();

	// Release GDI objects
    DeleteObject(font);
    DeleteDC(dc);

	// Create input layout
	const D3D12_INPUT_ELEMENT_DESC vertex_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Load vertex shader
	ComPtr<ID3DBlob> vtx = mRenderer->CreateVertexShader("Assets/Shaders/FontVertexShader.hlsl");

	// Load pixel shader
	ComPtr<ID3DBlob> pix = mRenderer->CreatePixelShader("Assets/Shaders/FontPixelShader.hlsl");

	mPipelineState = mRenderer->CreatePipelineState(vtx.Get(), vertex_desc, ARRAYSIZE(vertex_desc), pix.Get(), D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, PipelineState::EDepthTest::Off, PipelineState::EBlendMode::AlphaTest, PipelineState::ECullMode::Backface);

	// Create texture
	mTexture = mRenderer->CreateTexture(surface);

	// Trace success
	Trace("Created font \"%s\" with height %d in a %dx%d surface", mFontName.c_str(), mCharHeight, mHorizontalTexels, mVerticalTexels);

	return true;
}

Float2 Font::MeasureText(const string_view &inText) const
{
	JPH_PROFILE("MeasureText");

	Float2 extents(0, 1.0f);

	// Current raster position
	float x = 0;

	// Loop through string
	for (uint i = 0; i < inText.size(); ++i)
	{
		// Get character
		int ch = inText[i];

		// Create character if it is printable
		static_assert(cBeginChar == ' ', "We skip space in the for loop below");
		if (ch > cBeginChar && ch < cEndChar)
		{
			// Update extents
			int c1 = ch - cBeginChar;
			extents.x = max(extents.x, x + float(mWidth[c1]) / mCharHeight);
		}

		// Go to next (x, y) location
		if (ch == '\n')
		{
			// Next line
			x = 0;
			extents.y += 1.0f;
		}
		else if (i + 1 < inText.size())
		{
			// Do spacing between the two characters
			int c1 = ch - cBeginChar;
			int c2 = inText[i + 1] - cBeginChar;

			if (c1 >= 0 && c1 < cNumChars && c2 >= 0 && c2 < cNumChars)
				x += float(mSpacing[c1][c2]) / mCharHeight;
		}
	}

	return extents;
}

bool Font::CreateString(Mat44Arg inTransform, const string_view &inText, ColorArg inColor, RenderPrimitive &ioPrimitive) const
{
	JPH_PROFILE("CreateString");

	// Reset primitive
	ioPrimitive.Clear();

	// Count the number of printable chars
	int printable = 0;
	for (uint i = 0; i < inText.size(); ++i)
	{
		int ch = inText[i];
		static_assert(cBeginChar == ' ', "We skip space in the for loop below");
		if (ch > cBeginChar && ch < cEndChar) // Space is not printable
			printable++;
	}
	if (printable == 0)
		return false;

	// Get correction factor for texture size
	float texel_to_u = 1.0f / mHorizontalTexels;
	float texel_to_v = 1.0f / mVerticalTexels;

	int vtx_size = printable * 4;
	int idx_size = printable * 6;
	ioPrimitive.CreateVertexBuffer(vtx_size, sizeof(FontVertex));
	ioPrimitive.CreateIndexBuffer(idx_size);

	// Current vertex
	uint32 vtx = 0;

	// Lock buffers
	FontVertex *font_vtx = (FontVertex *)ioPrimitive.LockVertexBuffer();
	uint32 *idx_start = ioPrimitive.LockIndexBuffer();
	uint32 *idx = idx_start;

	// Current raster position
	float x = 0, y = -1.0f;

	// Loop through string
	for (uint i = 0; i < inText.size(); ++i)
	{
		// Get character
		int ch = inText[i];

		// Create character if it is printable
		static_assert(cBeginChar == ' ', "We skip space in the for loop below");
		if (ch > cBeginChar && ch < cEndChar)
		{
			// Get index for character
			int c1 = ch - cBeginChar;

			// Create indices
			*idx = vtx;
			++idx;
			*idx = vtx + 3;
			++idx;
			*idx = vtx + 1;
			++idx;
			*idx = vtx;
			++idx;
			*idx = vtx + 2;
			++idx;
			*idx = vtx + 3;
			++idx;
			vtx += 4;

			// Get properties of this character
			Float2 uv_start(texel_to_u * mStartU[c1], texel_to_v * mStartV[c1]);
			Float2 uv_end(texel_to_u * (mStartU[c1] + mWidth[c1]), texel_to_v * (mStartV[c1] + mCharHeight));
			Float2 xy_end(x + float(mWidth[c1]) / mCharHeight, y + 1.0f);

			// Create vertices
			(inTransform * Vec3(x, y, 0)).StoreFloat3(&font_vtx->mPosition);
			font_vtx->mColor = inColor;
			font_vtx->mTexCoord = Float2(uv_start.x, uv_end.y);
			++font_vtx;

			(inTransform * Vec3(x, xy_end.y, 0)).StoreFloat3(&font_vtx->mPosition);
			font_vtx->mColor = inColor;
			font_vtx->mTexCoord = uv_start;
			++font_vtx;

			(inTransform * Vec3(xy_end.x, y, 0)).StoreFloat3(&font_vtx->mPosition);
			font_vtx->mColor = inColor;
			font_vtx->mTexCoord = uv_end;
			++font_vtx;

			(inTransform * Vec3(xy_end.x, xy_end.y, 0)).StoreFloat3(&font_vtx->mPosition);
			font_vtx->mColor = inColor;
			font_vtx->mTexCoord = Float2(uv_end.x, uv_start.y);
			++font_vtx;
		}

		// Go to next (x, y) location
		if (ch == '\n')
		{
			// Next line
			x = 0.0f;
			y -= 1.0f;
		}
		else if (i + 1 < inText.size())
		{
			// Do spacing between the two characters
			int c1 = ch - cBeginChar;
			int c2 = inText[i + 1] - cBeginChar;

			if (c1 >= 0 && c1 < cNumChars && c2 >= 0 && c2 < cNumChars)
				x += float(mSpacing[c1][c2]) / mCharHeight;
		}
	}

	// Check that we completely filled the output buffer
	JPH_ASSERT(vtx == (uint32)vtx_size);
	JPH_ASSERT(idx == idx_start + idx_size);

	// Unlock buffers
	ioPrimitive.UnlockVertexBuffer();
	ioPrimitive.UnlockIndexBuffer();

	return true;
}

void Font::DrawText3D(Mat44Arg inTransform, const string_view &inText, ColorArg inColor) const
{
	JPH_PROFILE("DrawText3D");

	// Early out
	if (inText.empty())
		return;

	RenderPrimitive primitive(mRenderer, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if (CreateString(inTransform, inText, inColor, primitive))
	{
		mTexture->Bind(2);

		mPipelineState->Activate();

		primitive.Draw();
	}
}
