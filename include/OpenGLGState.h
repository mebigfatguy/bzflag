/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* OpenGLGState:
 *	Encapsulates OpenGL rendering state information.
 */

#ifndef BZF_OPENGL_GSTATE_H
#define BZF_OPENGL_GSTATE_H

#include "common.h"
#include "OpenGLTexture.h"
#include "CallbackList.h"

class OpenGLGStateRep;

typedef void			(*GraphicsContextInitializer)(
							bool destroy, void* userData);

//
// GStateRep -- not for public consumption
//

class GState {
public:
	enum TexEnv { kModulate, kDecal, kReplace };
	enum Shading { kConstant, kLinear };
	enum Func { kNever, kAlways, kEqual, kNotEqual,
						kLess, kLEqual, kGreater, kGEqual };
	enum BlendFactor { kZero, kOne,
						kSrcAlpha, kOneMinusSrcAlpha,
						kDstAlpha, kOneMinusDstAlpha,
						kSrcColor, kOneMinusSrcColor,
						kDstColor, kOneMinusDstColor };

	GState();

public:
	// gstate builders should not modify these after freezing the gstate rep
	OpenGLTexture		texture;
	TexEnv				texEnv;
	Shading				shadingModel;
	BlendFactor			blendingSrc;
	BlendFactor			blendingDst;
	bool				smoothing;
	bool				culling;
	Func				alphaFunc;
	float				alphaFuncRef;
	Func				depthFunc;
	bool				depthMask;
	float				pointSize;
	int					pass;
};

//
// OpenGLGState -- a bundle of rendering state
//

class OpenGLGState {
public:
	OpenGLGState();
	OpenGLGState(const OpenGLGState&);
	OpenGLGState(OpenGLGStateRep*);
	~OpenGLGState();
	OpenGLGState&		operator=(const OpenGLGState& state);

	// set the device and shadow rendering state to a known state.
	// this assumes the shadow state may not accurately reflect the
	// device state.
	static void			init();

	// reset the device and shadow rendering state to a known state.
	// this assumes the shadow rendering state correctly reflects
	// the device state.  use init() is the device state has been
	// modified externally.
	static void			resetState();

	// change the device and shadow rendering state to match this
	// object.  only the state that's different from the shadow
	// state is changed.  direct changes to the device state made
	// by the client are not detected.  if any have been made then
	// setState() may not yield the correct device state.
	void				setState() const;

	// get the GState
	const GState*		getState() const;

	// instrumentation methods.
	struct Instruments {
	public:
		float			time;
		unsigned int	nState;
		unsigned int	nTexture;
		unsigned int	nTexEnv;
		unsigned int	nShading;
		unsigned int	nBlending;
		unsigned int	nSmoothing;
		unsigned int	nCulling;
		unsigned int	nAlphaFunc;
		unsigned int	nDepthFunc;
		unsigned int	nDepthMask;
		unsigned int	nPointSize;
	};
	static void					instrReset();
	static const Instruments*	instrGet();

	// these are in OpenGLGState for lack of a better place.  register...
	// is for clients to add a function to call when the graphics context
	// has been destroyed and must be recreated.  freeContext() invokes
	// all the callbacks in the reverse order they were registered with
	// the destroy parameter set to true;  the callbacks should release
	// all graphics state.  initContext() invokes all the callbacks in the
	// order they were registered with the destroy parameter set to
	// false;  the callbacks may initialize OpenGL state.  initContext()
	// then resets the rendering state to the default.
	//
	// the callbacks must not add or remove context initializer callbacks.
	//
	// destroying and recreating the OpenGL context is only necessary on
	// platforms that cannot abstract the graphics system sufficiently.
	// for example, on win32, changing the display bit depth will cause
	// most OpenGL drivers to crash unless we destroy the context before
	// the switch and recreate it afterwards.
	static void			addContextInitializer(
							GraphicsContextInitializer,
							void* userData = NULL);
	static void			removeContextInitializer(
							GraphicsContextInitializer,
							void* userData = NULL);
	static void			freeContext();
	static void			initContext();

private:
	// builder needs access to rep to construct from an OpenGLGState
	friend class OpenGLGStateBuilder;
	OpenGLGStateRep*	rep;
	static CallbackList<GraphicsContextInitializer>	initList;
};

//
// GState builder -- creates gstates
//

class OpenGLGStateBuilder {
public:
	OpenGLGStateBuilder();
	OpenGLGStateBuilder(const OpenGLGState&);
	~OpenGLGStateBuilder();
	OpenGLGStateBuilder &operator=(const OpenGLGState&);

	// set the default state;  the defaults are the default arguments
	// to each method to set state.
	void				reset();

	// an invalid (default) texture disables texturing
	void				setTexture(const OpenGLTexture&);
	void				setTexEnv(GState::TexEnv = GState::kModulate);
	void				setShading(GState::Shading = GState::kConstant);
	void				setBlending(
							GState::BlendFactor src = GState::kOne,
							GState::BlendFactor dst = GState::kZero);
	void				setSmoothing(bool = false);
	void				setCulling(bool = true);
	void				setAlphaFunc(GState::Func = GState::kAlways,
							float ref = 0.0f);
	void				setDepthFunc(GState::Func = GState::kAlways);
	void				setDepthMask(bool enabled = true);
	void				setPointSize(float size = 1.0f);
	void				setPass(int = 0);

	// get current builder state
	OpenGLTexture		getTexture() const;
	GState::TexEnv		getTexEnv() const;
	GState::Shading		getShading() const;
	GState::BlendFactor	getBlendingSrc() const;
	GState::BlendFactor	getBlendingDst() const;
	bool				getSmoothing() const;
	bool				getCulling() const;
	GState::Func		getAlphaFunc() const;
	float				getAlphaFuncRef() const;
	GState::Func		getDepthFunc() const;
	bool				getDepthMask() const;
	float				getPointSize() const;
	int					getPass() const;

	// return a gstate having the current state of the builder
	OpenGLGState		getState() const;

private:
	GState*				data;
	OpenGLGStateRep*	rep;
};

inline
void					OpenGLGStateBuilder::setTexture(
								const OpenGLTexture& texture)
{
	data->texture = texture;
}

inline
void					OpenGLGStateBuilder::setTexEnv(
								GState::TexEnv texEnv)
{
	data->texEnv = texEnv;
}

inline
void					OpenGLGStateBuilder::setShading(
								GState::Shading model)
{
	data->shadingModel = model;
}

inline
void					OpenGLGStateBuilder::setBlending(
								GState::BlendFactor src,
								GState::BlendFactor dst)
{
	data->blendingSrc = src;
	data->blendingDst = dst;
}

inline
void					OpenGLGStateBuilder::setSmoothing(bool enabled)
{
	data->smoothing = enabled;
}

inline
void					OpenGLGStateBuilder::setCulling(bool enabled)
{
	data->culling = enabled;
}

inline
void					OpenGLGStateBuilder::setAlphaFunc(
								GState::Func func, float ref)
{
	data->alphaFunc    = func;
	data->alphaFuncRef = ref;
}

inline
void					OpenGLGStateBuilder::setDepthFunc(
								GState::Func func)
{
	data->depthFunc = func;
}

inline
void					OpenGLGStateBuilder::setDepthMask(bool enabled)
{
	data->depthMask = enabled;
}

inline
void					OpenGLGStateBuilder::setPointSize(float size)
{
	data->pointSize = size;
}

inline
void					OpenGLGStateBuilder::setPass(int pass)
{
	data->pass = pass;
}

inline
OpenGLTexture			OpenGLGStateBuilder::getTexture() const
{
	return data->texture;
}

inline
GState::TexEnv			OpenGLGStateBuilder::getTexEnv() const
{
	return data->texEnv;
}

inline
GState::Shading			OpenGLGStateBuilder::getShading() const
{
	return data->shadingModel;
}

inline
GState::BlendFactor		OpenGLGStateBuilder::getBlendingSrc() const
{
	return data->blendingSrc;
}

inline
GState::BlendFactor		OpenGLGStateBuilder::getBlendingDst() const
{
	return data->blendingDst;
}

inline
bool					OpenGLGStateBuilder::getSmoothing() const
{
	return data->smoothing;
}

inline
bool					OpenGLGStateBuilder::getCulling() const
{
	return data->culling;
}

inline
GState::Func			OpenGLGStateBuilder::getAlphaFunc() const
{
	return data->alphaFunc;
}

inline
float					OpenGLGStateBuilder::getAlphaFuncRef() const
{
	return data->alphaFuncRef;
}

inline
GState::Func			OpenGLGStateBuilder::getDepthFunc() const
{
	return data->depthFunc;
}

inline
bool					OpenGLGStateBuilder::getDepthMask() const
{
	return data->depthMask;
}

inline
float					OpenGLGStateBuilder::getPointSize() const
{
	return data->pointSize;
}

inline
int						OpenGLGStateBuilder::getPass() const
{
	return data->pass;
}

#endif // BZF_OPENGL_GSTATE_H
