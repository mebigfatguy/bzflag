/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* EighthDBoxSceneNode:
 *	Encapsulates information for rendering the eighth dimension
 *	of a pyramid building.
 */

#ifndef	BZF_EIGHTHD_PYR_SCENE_NODE_H
#define	BZF_EIGHTHD_PYR_SCENE_NODE_H

#include "EighthDimSceneNode.h"

class EighthDPyrSceneNode : public EighthDimSceneNode {
  public:
			EighthDPyrSceneNode(const float pos[3],
					const float size[3], float rotation);
			~EighthDPyrSceneNode();

    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);

  protected:
    class EighthDPyrRenderNode : public RenderNode {
      public:
			EighthDPyrRenderNode(const EighthDPyrSceneNode*,
				const float pos[3],
				const float size[3], float rotation);
			~EighthDPyrRenderNode();
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      private:
	const EighthDPyrSceneNode* sceneNode;
	GLfloat		corner[5][3];
    };

  private:
    OpenGLGState	 gstate;
    EighthDPyrRenderNode renderNode;
};

#endif // BZF_EIGHTHD_PYR_SCENE_NODE_H
