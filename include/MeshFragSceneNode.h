/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* MeshFragSceneNode:
 *	Encapsulates information for rendering a mesh fragment
 *      (a collection of faces with the same material properties).
 *	Does not support level of detail.
 */

#ifndef	BZF_MESH_FRAG_SCENE_NODE_H
#define	BZF_MESH_FRAG_SCENE_NODE_H

#include "common.h"
#include "WallSceneNode.h"
#include "BzMaterial.h"

//
// NOTES:
//
// - Make sure that "noPlane" is set to true, for Mesh Fragments can not be
//   used as occluders, and can not be culled as a simple plane
//
// - The lists are all GL_TRIANGLE lists
//       

class MeshFace;

class MeshFragSceneNode : public WallSceneNode {

  public:
    MeshFragSceneNode(int faceCount, const MeshFace** faces);
    ~MeshFragSceneNode();

    // virtual functions from SceneNode
    bool cull(const ViewFrustum&) const;
    void addShadowNodes(SceneRenderer&);
    void addRenderNodes(SceneRenderer&);

    // virtual functions from WallSceneNode
    void getExtents(float* mins, float* maxs) const;
    bool inAxisBox(const float* mins, const float* maxs) const;
      
  protected:
    class Geometry : public RenderNode {
      public:
        Geometry(MeshFragSceneNode* node, bool shadow);
        ~Geometry();
        
	void render();
	void setStyle(int _style) { style = _style; }
	const GLfloat* getPosition() { return sceneNode->getSphere(); }
	
      private:
	void drawV() const; // draw with just vertices
	void drawVT() const; // draw with texcoords
	void drawVN() const; // draw with normals
	void drawVTN() const; // draw with texcoords and normals

      private:
	int style;
	bool isShadow; 
	MeshFragSceneNode* sceneNode;
    };

  private:
    Geometry* renderNode;
    Geometry* shadowNode;
    
    GLint faceCount;
    const MeshFace** faces;
    
    GLint arrayCount;
    GLfloat* vertices;
    GLfloat* normals;
    GLfloat* texcoords;

    float mins[3], maxs[3];

  friend class MeshFragSceneNode::Geometry;
};


#endif // BZF_MESH_FRAG_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

