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

#include "ContactSurfaceIntersector.h"
#include "Body.h"

//
// ContactSurfaceIntersectorManager
//

ContactSurfaceIntersectorManager*
						ContactSurfaceIntersectorManager::mgr = NULL;

ContactSurfaceIntersectorManager::ContactSurfaceIntersectorManager()
{
	// do nothing
}

ContactSurfaceIntersectorManager::~ContactSurfaceIntersectorManager()
{
	// clean up
	for (IntersectorMap::iterator indexA = intersectors.begin();
								indexA != intersectors.end(); ++indexA) {
		MapB& mapB = indexA->second;
		for (MapB::iterator indexB = mapB.begin();
								indexB != mapB.end(); ++indexB)
			delete indexB->second;
	}

	mgr = NULL;
}

ContactSurfaceIntersectorManager*
						ContactSurfaceIntersectorManager::getInstance()
{
	if (mgr == NULL)
		mgr = new ContactSurfaceIntersectorManager;
	return mgr;
}

void					ContactSurfaceIntersectorManager::add(
								const void* typeA, const void* typeB,
								ContactSurfaceIntersectorBase* adopted)
{
	// look up map for typeA
	IntersectorMap::iterator indexA = intersectors.find(typeA);
	if (indexA == intersectors.end())
		indexA = intersectors.insert(std::make_pair(typeA, MapB())).first;

	// look up intersector for typeB
	MapB::iterator indexB = indexA->second.find(typeB);
	if (indexB == indexA->second.end()) {
		// it's new
		indexA->second.insert(std::make_pair(typeB, adopted));
	}
	else {
		// replace previous
		delete indexB->second;
		indexB->second = adopted;
	}
}

void					ContactSurfaceIntersectorManager::remove(
								const void* typeA, const void* typeB)
{
	// look up map for typeA
	IntersectorMap::iterator indexA = intersectors.find(typeA);
	if (indexA == intersectors.end())
		return;

	// look up intersector for typeB
	MapB::iterator indexB = indexA->second.find(typeB);
	if (indexB == indexA->second.end())
		return;

	// remove
	delete indexB->second;
	indexA->second.erase(indexB);
}

const ContactSurfaceIntersectorBase*
						ContactSurfaceIntersectorManager::get(
								const void* typeA, const void* typeB) const
{
	// look up map for typeA
	IntersectorMap::const_iterator indexA = intersectors.find(typeA);
	if (indexA == intersectors.end())
		return NULL;

	// look up intersector for typeB
	MapB::const_iterator indexB = indexA->second.find(typeB);
	if (indexB == indexA->second.end())
		return NULL;

	return indexB->second;
}

bool					ContactSurfaceIntersectorManager::intersect(
								ContactPoints& contacts,
								Body* a,
								Body* b,
								ContactSurface* aSurface,
								ContactSurface* bSurface) const
{
	const unsigned int start = contacts.size();

	// get the intersector that matches the types
	const ContactSurfaceIntersectorBase* intersector =
								get(aSurface->getType(), bSurface->getType());
	if (intersector != NULL) {
		// transform surfaces to a's coordinate system.
		aSurface->transform(a->getInverseTransform());
		bSurface->transform(a->getInverseTransform());

		// intersect (a,b)
		intersector->intersect(contacts, a, b, aSurface, bSurface);

		// transform to world space and set a and b
		const Matrix& pXForm = a->getTransform();
		const Matrix& nXForm = a->getInverseTransposeTransform();
		const unsigned int end = contacts.size();
		for (unsigned int i = start; i < end; ++i) {
			contacts[i].point.xformPoint(pXForm);
			contacts[i].normal = nXForm * contacts[i].normal;
		}

		return true;
	}

	// try swapping the surfaces
	intersector = get(bSurface->getType(), aSurface->getType());
	if (intersector != NULL) {
		// transform surfaces to b's coordinate system.
		aSurface->transform(b->getInverseTransform());
		bSurface->transform(b->getInverseTransform());

		// intersect (b,a)
		intersector->intersect(contacts, b, a, bSurface, aSurface);

		// flip normals and transform to world space and set a and b
		const Matrix& pXForm = b->getTransform();
		const Matrix& nXForm = b->getInverseTransposeTransform();
		const unsigned int end = contacts.size();
		for (unsigned int i = start; i < end; ++i) {
			contacts[i].point.xformPoint(pXForm);
			contacts[i].normal = nXForm * contacts[i].normal;
		}

		return true;
	}

	return false;
}
// ex: shiftwidth=4 tabstop=4
