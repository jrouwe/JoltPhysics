// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Image/ZoomImage.h>
#include <Image/Surface.h>
#include <Image/BlitSurface.h>
#include <Jolt/Core/Profiler.h>

//////////////////////////////////////////////////////////////////////////////////////////
// ImageFilter
//
// Abstract class and some implementations of a filter, essentially an 1D weighting function
// which is not zero for t e [-GetSupport(), GetSupport()] and zero for all other t
// The integrand is usually 1 although it is not required for this implementation,
// since the filter is renormalized when it is sampled.
//////////////////////////////////////////////////////////////////////////////////////////

class ImageFilter
{
public:
	// Destructor
	virtual			~ImageFilter() = default;

	// Get support of this filter (+/- the range the filter function is not zero)
	virtual float	GetSupport() const = 0;						

	// Sample filter function at a certain point
	virtual float	GetValue(float t) const = 0;				
};																
								
class ImageFilterBox : public ImageFilter
{
	virtual float GetSupport() const override
	{ 
		return 0.5f;
	}

	virtual float GetValue(float t) const override
	{
		if (abs(t) <= 0.5f) 
			return 1.0f;
		else
			return 0.0f;
	}
};

class ImageFilterTriangle : public ImageFilter
{
	virtual float GetSupport() const override
	{ 
		return 1.0f;
	}

	virtual float GetValue(float t) const override
	{
		t = abs(t);

		if (t < 1.0f) 
			return 1.0f - t;
		else
			return 0.0f;
	}
};

class ImageFilterBell : public ImageFilter
{
	virtual float GetSupport() const override
	{ 
		return 1.5f;
	}

	virtual float GetValue(float t) const override
	{
		t = abs(t);

		if (t < 0.5f) 
			return 0.75f - t * t;		
		else if (t < 1.5f) 
		{
			t = t - 1.5f;
			return 0.5f * t * t;
		}
		else
			return 0.0f;
	}
};

class ImageFilterBSpline : public ImageFilter
{
	virtual float GetSupport() const override
	{ 
		return 2.0f;
	}

	virtual float GetValue(float t) const override
	{
		t = abs(t);

		if (t < 1.0f) 
		{
			float tt = t * t;
			return (0.5f * tt * t) - tt + (2.0f / 3.0f);
		} 
		else if (t < 2.0f) 
		{
			t = 2.0f - t;
			return (1.0f / 6.0f) * (t * t * t);
		}
		else
			return 0.0f;
	}
};

class ImageFilterLanczos3 : public ImageFilter
{
	virtual float GetSupport() const override
	{ 
		return 3.0f;
	}

	virtual float GetValue(float t) const override
	{
		t = abs(t);

		if (t < 3.0f) 
			return Sinc(t) * Sinc(t / 3.0f);
		else
			return 0.0f;
	}

private:
	static float Sinc(float x)
	{
		x *= JPH_PI;

		if (abs(x) < 1.0e-5f)
			return 1.0f;

		return sin(x) / x;		
	}

};

class ImageFilterMitchell : public ImageFilter
{
	virtual float GetSupport() const override
	{ 
		return 2.0f;
	}

	virtual float GetValue(float t) const override
	{
		float tt = t * t;
		t = abs(t);

		if (t < 1.0f) 
			return (7.0f * (t * tt) - 12.0f * tt + (16.0f / 3.0f)) / 6.0f;
		else if (t < 2.0f) 
			return ((-7.0f / 3.0f) * (t * tt) + 12.0f * tt + -20.0f * t + (32.0f / 3.0f)) / 6.0f;
		else
			return 0.0f;
	}
};

static const ImageFilter &GetFilter(EFilter inFilter)
{
	static ImageFilterBox box;
	static ImageFilterTriangle triangle;
	static ImageFilterBell bell;
	static ImageFilterBSpline bspline;
	static ImageFilterLanczos3 lanczos3;
	static ImageFilterMitchell mitchell;

	switch (inFilter)
	{
		case FilterBox:								return box;
		case FilterTriangle:						return triangle;
		case FilterBell:							return bell;
		case FilterBSpline:							return bspline;
		case FilterLanczos3:						return lanczos3;
		case FilterMitchell:						return mitchell;
		default:				JPH_ASSERT(false);	return mitchell;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// ZoomSettings
//////////////////////////////////////////////////////////////////////////////////////////

const ZoomSettings ZoomSettings::sDefault;

ZoomSettings::ZoomSettings() : 
	mFilter(FilterMitchell), 
	mWrapFilter(true), 
	mBlur(1.0f)
{ 
}

bool ZoomSettings::operator == (const ZoomSettings &inRHS) const
{ 
	return mFilter == inRHS.mFilter 
		&& mWrapFilter == inRHS.mWrapFilter 
		&& mBlur == inRHS.mBlur;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Resizing a surface
//////////////////////////////////////////////////////////////////////////////////////////

// Structure used for zooming
struct Contrib
{
	int			mOffset;				// Offset of this pixel (relative to start of scanline)
	int			mWeight;				// Weight of this pixel in 0.12 fixed point format
};

static void sPrecalculateFilter(const ZoomSettings &inZoomSettings, int inOldLength, int inNewLength, int inOffsetFactor, vector<vector<Contrib>> &outContrib)
{
	JPH_PROFILE("PrecalculateFilter");

	// Get filter
	const ImageFilter &filter = GetFilter(inZoomSettings.mFilter);

	// Get scale
	float scale = float(inNewLength) / inOldLength;

	float fwidth, fscale;
	if (scale < 1.0f)
	{
		// Minify, broaden filter
		fwidth = filter.GetSupport() / scale;
		fscale = scale;
	}
	else
	{
		// Enlarge, filter is always used as is
		fwidth = filter.GetSupport();
		fscale = 1.0f;
	}

	// Adjust filter for blur
	fwidth *= inZoomSettings.mBlur;
	fscale /= inZoomSettings.mBlur;
	float min_fwidth = 1.0f;
	if (fwidth < min_fwidth)
	{
		fwidth = min_fwidth;
		fscale = filter.GetSupport() / min_fwidth;
	}

	// Make room for a whole scanline
	outContrib.resize(inNewLength);

	// Loop over the whole scanline
	for (int i = 0; i < inNewLength; ++i) 
	{
		// Compute center and left- and rightmost pixels affected
		float center = float(i) / scale;
		int left = int(floor(center - fwidth));
		int right = int(ceil(center + fwidth));

		// Reserve required elements
		vector<Contrib> &a = outContrib[i];
		a.reserve(right - left + 1);		

		// Total sum of all weights, for renormalization of the filter
		int filter_sum = 0;

		// Compute the contributions for each
		for (int source = left; source <= right; ++source) 
		{
			Contrib c;

			// Initialize the offset
			c.mOffset = source;

			// Compute weight at this position in 0.12 fixed point
			c.mWeight = int(4096.0f * filter.GetValue(fscale * (center - source)));
			if (c.mWeight == 0) continue;
			
			// Add weight to filter total
			filter_sum += c.mWeight;	

			// Reflect the filter at the edges if the filter is not to be wrapped (clamp)				
			if (!inZoomSettings.mWrapFilter && (c.mOffset < 0 || c.mOffset >= inOldLength))
				c.mOffset = -c.mOffset - 1;

			// Wrap the offset so that it falls within the image
			c.mOffset = (c.mOffset % inOldLength + inOldLength) % inOldLength;

			// Check that the offset falls within the image
			JPH_ASSERT(c.mOffset >= 0 && c.mOffset < inOldLength);

			// Multiply the offset with the specified factor
			c.mOffset *= inOffsetFactor;
			
			// Add the filter element
			a.push_back(c);
		}
		
		// Normalize the filter to 0.12 fixed point
		if (filter_sum != 0)
			for (uint n = 0; n < a.size(); ++n)
				a[n].mWeight = (a[n].mWeight * 4096) / filter_sum;
	}
}

static void sZoomHorizontal(RefConst<Surface> inSrc, Ref<Surface> ioDst, const ZoomSettings &inZoomSettings)
{	
	JPH_PROFILE("ZoomHorizontal");

	// Check zoom parameters
	JPH_ASSERT(inSrc->GetHeight() == ioDst->GetHeight());
	JPH_ASSERT(inSrc->GetFormat() == ioDst->GetFormat());

	const int width = ioDst->GetWidth();
	const int height = ioDst->GetHeight();
	const int components = ioDst->GetNumberOfComponents();
	const int delta_s = -components;
	const int delta_d = ioDst->GetBytesPerPixel() - components;

	// Pre-calculate filter contributions for a row
	vector<vector<Contrib>> contrib;	
	sPrecalculateFilter(inZoomSettings, inSrc->GetWidth(), ioDst->GetWidth(), inSrc->GetBytesPerPixel(), contrib);

	// Do the zoom
	for (int y = 0; y < height; ++y) 
	{
		const uint8 *s = inSrc->GetScanLine(y);
		uint8 *d = ioDst->GetScanLine(y);

		for (int x = 0; x < width; ++x) 
		{
			const vector<Contrib> &line = contrib[x];
			const size_t line_size_min_one = line.size() - 1;

			int c = components;
			do
			{
				int pixel = 0;

				// Apply the filter for one color component
				size_t j = line_size_min_one;
				do
				{
					const Contrib &cmp = line[j];
					pixel += cmp.mWeight * s[cmp.mOffset];
				}
				while (j--);

				// Clamp the pixel value
				if (pixel <= 0)
					*d = 0;
				else if (pixel >= (255 << 12))
					*d = 255;
				else
					*d = uint8(pixel >> 12);

				++s;
				++d;
			}
			while (--c);

			// Skip unused components if there are any
			s += delta_s;
			d += delta_d;
		}
	}
}

static void sZoomVertical(RefConst<Surface> inSrc, Ref<Surface> ioDst, const ZoomSettings &inZoomSettings)
{	
	JPH_PROFILE("ZoomVertical");

	// Check zoom parameters
	JPH_ASSERT(inSrc->GetWidth() == ioDst->GetWidth());
	JPH_ASSERT(inSrc->GetFormat() == ioDst->GetFormat());

	const int width = ioDst->GetWidth();
	const int height = ioDst->GetHeight();
	const int components = ioDst->GetNumberOfComponents();
	const int delta_s = inSrc->GetBytesPerPixel() - components;
	const int delta_d = ioDst->GetBytesPerPixel() - components;
	
	// Pre-calculate filter contributions for a row
	vector<vector<Contrib>> contrib;	
	sPrecalculateFilter(inZoomSettings, inSrc->GetHeight(), ioDst->GetHeight(), inSrc->GetStride(), contrib);

	// Do the zoom
	for (int y = 0; y < height; ++y) 
	{
		const uint8 *s = inSrc->GetScanLine(0);
		uint8 *d = ioDst->GetScanLine(y);
		const vector<Contrib> &line = contrib[y];		
		const size_t line_size_min_one = line.size() - 1;

		for (int x = 0; x < width; ++x) 
		{
			int c = components;
			do
			{
				int pixel = 0;

				// Apply the filter for one color component
				size_t j = line_size_min_one;
				do
				{
					const Contrib &cmp = line[j];	
					pixel += cmp.mWeight * s[cmp.mOffset];
				}
				while (j--);

				// Clamp the pixel value
				if (pixel <= 0)
					*d = 0;
				else if (pixel >= (255 << 12))
					*d = 255;
				else
					*d = uint8(pixel >> 12);

				++s;
				++d;
			}
			while (--c);

			// Skip unused components if there are any
			s += delta_s;
			d += delta_d;
		}
	}
}

bool ZoomImage(RefConst<Surface> inSrc, Ref<Surface> ioDst, const ZoomSettings &inZoomSettings)
{
	JPH_PROFILE("ZoomImage");

	// Get filter
	const ImageFilter &filter = GetFilter(inZoomSettings.mFilter);
	
	// Determine the temporary format that will require the least amount of components to be zoomed and the least amount of bytes pushed around
	ESurfaceFormat tmp_format;
	ESurfaceFormat src_format = inSrc->GetClosest8BitFormat();
	ESurfaceFormat dst_format = ioDst->GetClosest8BitFormat();
	const FormatDescription &src_desc = GetFormatDescription(src_format);
	const FormatDescription &dst_desc = GetFormatDescription(dst_format);
	if (src_desc.GetNumberOfComponents() < dst_desc.GetNumberOfComponents())
		tmp_format = src_format;
	else if (src_desc.GetNumberOfComponents() > dst_desc.GetNumberOfComponents())
		tmp_format = dst_format;
	else if (src_desc.GetBytesPerPixel() < dst_desc.GetBytesPerPixel())
		tmp_format = src_format;
	else
		tmp_format = dst_format;

	// Create temporary source buffer if nessecary
	RefConst<Surface> src = inSrc;
	if (inSrc->GetFormat() != tmp_format)
	{
		Ref<Surface> tmp = new SoftwareSurface(inSrc->GetWidth(), inSrc->GetHeight(), tmp_format);
		if (!BlitSurface(inSrc, tmp))
			return false;
		src = tmp;		
	}

	// Create temporary destination buffer if nessecary
	Ref<Surface> dst = ioDst;
	if (ioDst->GetFormat() != tmp_format)
		dst = new SoftwareSurface(ioDst->GetWidth(), ioDst->GetHeight(), tmp_format);

	src->Lock(ESurfaceLockMode::Read);
	dst->Lock(ESurfaceLockMode::Write);

	if (src->GetWidth() == dst->GetWidth())
	{
		// Only vertical zoom required
		sZoomVertical(src, dst, inZoomSettings);
	}
	else if (src->GetHeight() == dst->GetHeight())
	{
		// Only horizontal zoom required
		sZoomHorizontal(src, dst, inZoomSettings);
	}
	else
	{
		// Determine most optimal order
		float operations_vh = float(dst->GetWidth()) * (filter.GetSupport() * src->GetHeight() + filter.GetSupport() * dst->GetHeight());
		float operations_hv = float(dst->GetHeight()) * (filter.GetSupport() * src->GetWidth() + filter.GetSupport() * dst->GetWidth());
		if (operations_vh < operations_hv)
		{
			// Create temporary buffer to hold the vertical scale
			Ref<Surface> tmp = new SoftwareSurface(src->GetWidth(), dst->GetHeight(), tmp_format);
			tmp->Lock(ESurfaceLockMode::ReadWrite);
			
			// First scale vertically then horizontally
			sZoomVertical(src, tmp, inZoomSettings);
			sZoomHorizontal(tmp, dst, inZoomSettings);

			tmp->UnLock();
		}
		else
		{
			// Create temporary buffer to hold the horizontal scale
			Ref<Surface> tmp = new SoftwareSurface(dst->GetWidth(), src->GetHeight(), tmp_format);
			tmp->Lock(ESurfaceLockMode::ReadWrite);
			
			// First scale horizontally then vertically
			sZoomHorizontal(src, tmp, inZoomSettings);
			sZoomVertical(tmp, dst, inZoomSettings);

			tmp->UnLock();
		}
	}

	src->UnLock();
	dst->UnLock();

	// Convert to destination if required
	if (dst != ioDst)
		if (!BlitSurface(dst, ioDst))
			return false;

	return true;
}
