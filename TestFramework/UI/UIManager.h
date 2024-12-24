// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UIElement.h>
#include <UI/UITexturedQuad.h>
#include <Renderer/Renderer.h>
#include <Renderer/PipelineState.h>
#include <memory>

class Font;

const float cActivateScreenTime = 0.2f;

/// Manager class that manages UIElements
class UIManager : public UIElement
{
public:
	/// Constructor
								UIManager(Renderer *inRenderer);
	virtual						~UIManager() override;

	/// Update elements
	virtual void				Update(float inDeltaTime) override;

	/// Draw elements
	virtual void				Draw() const override;

	/// Only one layer can be active, if you push a layer it will exit in the background and no longer be updated
	void						PushLayer();
	void						PopLayer();
	int							GetNumLayers() const								{ return (int)mInactiveElements.size() + 1; }
	void						SetDrawInactiveLayers(bool inDraw)					{ mDrawInactiveElements = inDraw; }

	/// Find element by ID
	virtual UIElement *			FindByID(int inID) override;

	/// Listeners
	void						SetListener(UIEventListener *inListener)			{ mListener = inListener; }
	UIEventListener *			GetListener() const									{ return mListener; }

	/// Actions
	void						SetDeactivatedAction(function<void()> inAction)		{ mDeactivatedAction = inAction; }

	/// Event handling (returns true if the event has been handled)
	virtual bool				HandleUIEvent(EUIEvent inEvent, UIElement *inSender) override;

	/// Current state
	enum EState
	{
		STATE_INVALID,
		STATE_ACTIVATING,
		STATE_ACTIVE,
		STATE_DEACTIVATING,
		STATE_DEACTIVE
	};

	void						SwitchToState(EState inState);
	EState						GetState() const									{ return mState; }

	/// Calculate max horizontal and vertical distance of elements to edge of screen
	void						GetMaxElementDistanceToScreenEdge(int &outMaxH, int &outMaxV);

	/// Access to the renderer
	Renderer *					GetRenderer()										{ return mRenderer; }

	/// Drawing
	void						DrawQuad(int inX, int inY, int inWidth, int inHeight, const UITexturedQuad &inQuad, ColorArg inColor);

	/// Draw a string in screen coordinates (assumes that the projection matrix has been set up correctly)
	void						DrawText(int inX, int inY, const string_view &inText, const Font *inFont, ColorArg inColor = Color::sWhite);

private:
	Renderer *					mRenderer;
	UIEventListener *			mListener;
	Array<UIElementVector>		mInactiveElements;
	bool						mDrawInactiveElements = true;
	unique_ptr<PipelineState>	mTextured;
	unique_ptr<PipelineState>	mUntextured;
	function<void()>			mDeactivatedAction;

	EState						mState;
	float						mStateTime;
};
