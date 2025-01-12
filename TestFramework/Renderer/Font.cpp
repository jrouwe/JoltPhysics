// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Font.h>
#include <Renderer/Renderer.h>
#include <Image/Surface.h>
#include <Utils/ReadData.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/ScopeExit.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
JPH_CLANG_SUPPRESS_WARNING("-Wreserved-identifier")
JPH_CLANG_SUPPRESS_WARNING("-Wzero-as-null-pointer-constant")
JPH_CLANG_SUPPRESS_WARNING("-Wcast-qual")
JPH_CLANG_SUPPRESS_WARNING("-Wimplicit-fallthrough")
JPH_CLANG_SUPPRESS_WARNING("-Wcomma")
JPH_CLANG_SUPPRESS_WARNING("-Wdouble-promotion")
#define STB_TRUETYPE_IMPLEMENTATION
#include <External/stb_truetype.h>
JPH_SUPPRESS_WARNINGS_STD_END

Font::Font(Renderer *inRenderer) :
	mRenderer(inRenderer)
{
}

bool Font::Create(const char *inFontName, int inCharHeight)
{
	JPH_PROFILE("Create");

	// Initialize
	mFontName = inFontName;
	mCharHeight = inCharHeight;
	mHorizontalTexels = 64;
	mVerticalTexels = 64;

	constexpr int cSpacingH = 2; // Number of pixels to put horizontally between characters
	constexpr int cSpacingV = 2; // Number of pixels to put vertically between characters

	// Read font data
	Array<uint8> font_data = ReadData((String("Fonts/") + inFontName + ".ttf").c_str());

	// Construct a font info
	stbtt_fontinfo font;
	if (!stbtt_InitFont(&font, font_data.data(), stbtt_GetFontOffsetForIndex(font_data.data(), 0)))
		return false;

	// Get the base line for the font
	float scale = stbtt_ScaleForPixelHeight(&font, float(mCharHeight));
	int ascent;
	stbtt_GetFontVMetrics(&font, &ascent, nullptr, nullptr);
	int baseline = int(ascent * scale);

	// Create surface for characters
	Ref<SoftwareSurface> surface = new SoftwareSurface(mHorizontalTexels, mVerticalTexels, ESurfaceFormat::L8);
	surface->Clear();
	surface->Lock(ESurfaceLockMode::Write);

	// Draw all printable characters
try_again:;
	int x = 0, y = 0;
	static_assert(cBeginChar == ' ', "We skip space in the for loop below");
	for (int c = cBeginChar + 1; c < cEndChar; ++c)
	{
		// Get index in the arrays
		int idx = c - cBeginChar;

		int w, h, xoff, yoff;
		unsigned char *bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, &xoff, &yoff);
		JPH_SCOPE_EXIT([bitmap]{ STBTT_free(bitmap, nullptr); });
		yoff = baseline + yoff;

		// Check if there is room on this line
		if (int(x + xoff + w + cSpacingH) > mHorizontalTexels)
		{
			// Next line
			x = 0;
			y += mCharHeight + cSpacingV;

			// Check if character fits
			if (y + mCharHeight + cSpacingV > mVerticalTexels)
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
		JPH_ASSERT(w <= 0xff);
		mStartU[idx] = uint16(x);
		mStartV[idx] = uint16(y);
		mWidth[idx] = uint8(w + 1);

		// Copy the character data
		for (int y2 = 0; y2 < h; ++y2)
		{
			uint8 *src = bitmap + y2 * w;
			uint8 *dst = surface->GetScanLine(y + yoff + y2) + x + xoff;
			memcpy(dst, src, w);
		}

		// Go to the next character
		x += w + cSpacingH;
	}

	// Calculate spacing between characters
	for (int idx1 = 0; idx1 < cNumChars; ++idx1)
		for (int idx2 = 0; idx2 < cNumChars; ++idx2)
		{
			int c1 = cBeginChar + idx1;
			int c2 = cBeginChar + idx2;

			int advance;
			stbtt_GetCodepointHMetrics(&font, c1, &advance, nullptr);
			int spacing = Clamp(int(scale * (advance + stbtt_GetCodepointKernAdvance(&font, c1, c2))), 0, 0xff);
			mSpacing[idx1][idx2] = (uint8)spacing;
		}

	// Unlock surface
	surface->UnLock();

	// Create input layout
	const PipelineState::EInputDescription vertex_desc[] =
	{
		PipelineState::EInputDescription::Position,
		PipelineState::EInputDescription::TexCoord,
		PipelineState::EInputDescription::Color
	};

	// Load vertex shader
	Ref<VertexShader> vtx = mRenderer->CreateVertexShader("FontVertexShader");

	// Load pixel shader
	Ref<PixelShader> pix = mRenderer->CreatePixelShader("FontPixelShader");

	mPipelineState = mRenderer->CreatePipelineState(vtx, vertex_desc, std::size(vertex_desc), pix, PipelineState::EDrawPass::Normal, PipelineState::EFillMode::Solid, PipelineState::ETopology::Triangle, PipelineState::EDepthTest::Off, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::Backface);

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

	Ref<RenderPrimitive> primitive = mRenderer->CreateRenderPrimitive(PipelineState::ETopology::Triangle);
	if (CreateString(inTransform, inText, inColor, *primitive))
	{
		mTexture->Bind();

		mPipelineState->Activate();

		primitive->Draw();
	}
}
