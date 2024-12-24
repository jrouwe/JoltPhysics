// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

/// Defines how primitives should be rendered
class PipelineState
{
public:
	/// Describes the input layout of the vertex shader
	enum class EInputDescription
	{
		Position,						///< 3 float position
		Color,							///< 4 uint8 color
		Normal,							///< 3 float normal
		TexCoord,						///< 2 float texture coordinate
		InstanceColor,					///< 4 uint8 per instance color
		InstanceTransform,				///< 4x4 float per instance transform
		InstanceInvTransform,			///< 4x4 float per instance inverse transform
	};

	/// In which draw pass to use this pipeline state
	enum class EDrawPass
	{
		Shadow,
		Normal
	};

	/// The type of topology to emit
	enum class ETopology
	{
		Triangle,
		Line
	};

	/// Fill mode of the triangles
	enum class EFillMode
	{
		Solid,
		Wireframe
	};

	/// If depth write / depth test is on
	enum class EDepthTest
	{
		Off,
		On
	};

	/// How to blend the pixel from the shader in the back buffer
	enum class EBlendMode
	{
		Write,
		AlphaBlend,
	};

	/// How to cull triangles
	enum class ECullMode
	{
		Backface,
		FrontFace,
	};

	/// Destructor
	virtual								~PipelineState() = default;

	/// Make this pipeline state active (any primitives rendered after this will use this state)
	virtual void						Activate() = 0;
};
