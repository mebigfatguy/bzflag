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

#ifndef BZF_SCENE_NODE_BASE_TRANSFORM_H
#define BZF_SCENE_NODE_BASE_TRANSFORM_H

#include "SceneNodeGroup.h"

class Matrix;
class SceneVisitorParams;

class SceneNodeBaseTransform : public SceneNodeGroup {
public:
	enum Type { ModelView, Projection, Texture };

	SceneNodeBaseTransform();

	// get the transformation.  modify currentXForm appropriately.
	virtual void		get(Matrix& currentXForm,
							const SceneVisitorParams&) = 0;

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// fields
	SceneNodeSFEnum		type;
	SceneNodeSFUInt		up;

protected:
	virtual ~SceneNodeBaseTransform();
};

#endif
