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

/* interface header */
#include "WorldInfo.h"

/* common implementation headers */
#include "global.h"
#include "Pack.h"
#include "Protocol.h"
#include "Intersect.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "BzMaterial.h"
#include "PhysicsDriver.h"

/* compression library header */
#include "../zlib/zlib.h"


WorldInfo::WorldInfo() :
  maxHeight(0),
  database(NULL)
{
  size[0] = 400.0f;
  size[1] = 400.0f;
  gravity = -9.81f;
  waterLevel = -1.0f;
  waterMatRef = NULL;
}

WorldInfo::~WorldInfo()
{
  delete[] database;
  unsigned int i;
  for (i = 0; i < walls.size(); i++) {
    delete walls[i];
  }
  for (i = 0; i < meshes.size(); i++) {
    if (!meshes[i]->getIsLocal()) {
      delete meshes[i];
    }
  }
  for (i = 0; i < arcs.size(); i++) {
    delete arcs[i];
  }
  for (i = 0; i < cones.size(); i++) {
    delete cones[i];
  }
  for (i = 0; i < spheres.size(); i++) {
    delete spheres[i];
  }
  for (i = 0; i < tetras.size(); i++) {
    delete tetras[i];
  }
  for (i = 0; i < boxes.size(); i++) {
    delete boxes[i];
  }
  for (i = 0; i < bases.size(); i++) {
    delete bases[i];
  }
  for (i = 0; i < pyramids.size(); i++) {
    delete pyramids[i];
  }
  for (i = 0; i < teleporters.size(); i++) {
    delete teleporters[i];
  }
}

void WorldInfo::addWall(float x, float y, float z, float r, float w, float h)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  WallObstacle* wall = new WallObstacle(pos, r, w, h);
  walls.push_back (wall);
}

void WorldInfo::addBox(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  BoxBuilding* box = new BoxBuilding(pos, r, w, d, h, drive, shoot, false);
  boxes.push_back (box);
}

void WorldInfo::addPyramid(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot, bool flipZ)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  PyramidBuilding* pyr = new PyramidBuilding(pos, r, w, d, h, drive, shoot);
  if (flipZ) {
    pyr->setZFlip();
  }
  pyramids.push_back (pyr);
}

void WorldInfo::addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b, bool horizontal, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  Teleporter* tele = new Teleporter(pos, r, w, d, h, b, horizontal, drive, shoot);
  teleporters.push_back (tele);

  // default to passthru linkage
  int index = teleporters.size() - 1;
  setTeleporterTarget ((index * 2) + 1, (index * 2));
  setTeleporterTarget ((index * 2), (index * 2) + 1);
}

void WorldInfo::addBase(float x, float y, float z, float r,
                        float w, float d, float h, int color,
                        bool /* drive */, bool /* shoot */)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  const float size[3] = {w, d, h};
  BaseBuilding* base = new BaseBuilding(pos, r, size, color);
  bases.push_back (base);
}

void WorldInfo::addLink(int from, int to)
{
  // discard links to/from teleporters that don't exist
  // note that -1 "to" means the client picks one at random
  if ((unsigned(from) > teleporters.size() * 2 + 1) ||
     ((unsigned(to) > teleporters.size() * 2 + 1) && (to != -1))) {
    DEBUG1("Warning: bad teleporter link dropped from=%d to=%d\n", from, to);
  } else {
    setTeleporterTarget (from, to);
  }
}

void WorldInfo::addMesh(MeshObstacle* mesh)
{
  float mins[3], maxs[3];
  mesh->getExtents(mins, maxs);
  if (maxs[2] > maxHeight) {
    maxHeight = maxs[2];
  }
  meshes.push_back(mesh);
}

void WorldInfo::addArc(ArcObstacle* arc)
{
  float mins[3], maxs[3];
  arc->getMesh()->getExtents(mins, maxs);
  if (maxs[2] > maxHeight) {
    maxHeight = maxs[2];
  }
  arcs.push_back(arc);
  arc->getMesh()->setIsLocal(true);
  meshes.push_back(arc->getMesh());
}

void WorldInfo::addCone(ConeObstacle* cone)
{
  float mins[3], maxs[3];
  cone->getMesh()->getExtents(mins, maxs);
  if (maxs[2] > maxHeight) {
    maxHeight = maxs[2];
  }
  cones.push_back(cone);
  cone->getMesh()->setIsLocal(true);
  meshes.push_back(cone->getMesh());
}

void WorldInfo::addSphere(SphereObstacle* sphere)
{
  float mins[3], maxs[3];
  sphere->getMesh()->getExtents(mins, maxs);
  if (maxs[2] > maxHeight) {
    maxHeight = maxs[2];
  }
  spheres.push_back(sphere);
  sphere->getMesh()->setIsLocal(true);
  meshes.push_back(sphere->getMesh());
}

void WorldInfo::addTetra(TetraBuilding* tetra)
{
  float mins[3], maxs[3];
  tetra->getMesh()->getExtents(mins, maxs);
  if (maxs[2] > maxHeight) {
    maxHeight = maxs[2];
  }
  tetras.push_back(tetra);
  tetra->getMesh()->setIsLocal(true);
  meshes.push_back(tetra->getMesh());
}

void WorldInfo::addZone(const CustomZone *zone)
{
  entryZones.addZone( zone );
}

void WorldInfo::addWeapon(const FlagType *type, const float *origin, float direction,
                          float initdelay, const std::vector<float> &delay, TimeKeeper &sync)
{
  worldWeapons.add(type, origin, direction, initdelay, delay, sync);
}

void WorldInfo::addWaterLevel (float level, const BzMaterial* matref)
{
  waterLevel = level;
  waterMatRef = matref;
}

void WorldInfo::makeWaterMaterial()
{
  // the texture matrix
  TextureMatrix* texmat = new TextureMatrix;
  texmat->setName("defaultWaterLevel");
  texmat->setShiftParams(0.05f, 0.0f);
  int texmatIndex = TEXMATRIXMGR.addMatrix(texmat);

  // the material
  BzMaterial material;
  const float diffuse[4] = {0.65f, 1.0f, 0.5f, 0.9f};
  material.reset();
  material.setName("defaultWaterLevel");
  material.setTexture("water");
  material.setTextureMatrix(texmatIndex); // generate a default later
  material.setDiffuse(diffuse);
  material.setUseTextureAlpha(true); // make sure that alpha is enabled
  material.setUseColorOnTexture(false); // only use the color as a backup
  material.setUseSphereMap(false);
  waterMatRef = MATERIALMGR.addMaterial(&material);

  return;  
}

float WorldInfo::getWaterLevel() const
{
  return waterLevel;
}

float WorldInfo::getMaxWorldHeight()
{
  return maxHeight;
}

void WorldInfo::setTeleporterTarget(int src, int tgt)
{
  if ((int)teleportTargets.size() < src+1)
    teleportTargets.resize(((src/2)+1)*2);

  // record target in source entry
  teleportTargets[src] = tgt;
}

WorldWeapons& WorldInfo::getWorldWeapons()
{
  return worldWeapons;
}

void                    WorldInfo::loadCollisionManager()
{
  collisionManager.load(meshes, boxes, bases, pyramids, teleporters);
  return;
}

void                    WorldInfo::checkCollisionManager()
{
  if (collisionManager.needReload()) {
    // reload the collision grid
    collisionManager.load(meshes, boxes, bases, pyramids, teleporters);
  }
  return;
}

bool WorldInfo::rectHitCirc(float dx, float dy, const float *p, float r) const
{
  // Algorithm from Graphics Gems, pp51-53.
  const float rr = r * r, rx = -p[0], ry = -p[1];
  if (rx + dx < 0.0f) // west of rect
    if (ry + dy < 0.0f) //  sw corner
      return (rx + dx) * (rx + dx) + (ry + dy) * (ry + dy) < rr;
    else if (ry - dy > 0.0f) //  nw corner
      return (rx + dx) * (rx + dx) + (ry - dy) * (ry - dy) < rr;
    else //  due west
      return rx + dx > -r;

  else if (rx - dx > 0.0f) // east of rect
    if (ry + dy < 0.0f) //  se corner
      return (rx - dx) * (rx - dx) + (ry + dy) * (ry + dy) < rr;
    else if (ry - dy > 0.0f) //  ne corner
      return (rx - dx) * (rx - dx) + (ry - dy) * (ry - dy) < rr;
    else //  due east
      return rx - dx < r;

  else if (ry + dy < 0.0f) // due south
    return ry + dy > -r;

  else if (ry - dy > 0.0f) // due north
    return ry - dy < r;

  // circle origin in rect
  return true;
}

bool WorldInfo::inRect(const float *p1, float angle, const float *size, 
                       float x, float y, float r) const
{
  // translate origin
  float pa[2];
  pa[0] = x - p1[0];
  pa[1] = y - p1[1];

  // rotate
  float pb[2];
  const float c = cosf(-angle), s = sinf(-angle);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];

  // do test
  return rectHitCirc(size[0], size[1], pb, r);
}


InBuildingType WorldInfo::inCylinderNoOctree(Obstacle **location,
                                             float x, float y, float z, float radius,
                                             float height)
{
  if (height < Epsilon) {
    height = Epsilon;
  }

  float pos[3] = {x, y, z};

  for (std::vector<BaseBuilding*>::iterator base_it = bases.begin();
       base_it != bases.end(); ++base_it) {
    BaseBuilding* base = *base_it;
    if (base->inCylinder(pos, radius, height)) {
      if (location != NULL) {
        *location = base;
      }
      return IN_BASE;
    }
  }
  for (std::vector<BoxBuilding*>::iterator box_it = boxes.begin();
       box_it != boxes.end(); ++box_it) {
    BoxBuilding* box = *box_it;
    if (box->inCylinder(pos, radius, height)) {
      if (location != NULL) {
        *location = box;
      }
      if (box->isDriveThrough()) {
        return IN_BOX_DRIVETHROUGH;
      }
      else {
        return IN_BOX_NOTDRIVETHROUGH;
      }
    }
  }
  for (std::vector<PyramidBuilding*>::iterator pyr_it = pyramids.begin();
       pyr_it != pyramids.end(); ++pyr_it) {
    PyramidBuilding* pyr = *pyr_it;
    if (pyr->inCylinder(pos, radius, height)) {
      if (location != NULL) {
        *location = pyr;
      }
      return IN_PYRAMID;
    }
  }
  for (std::vector<TetraBuilding*>::iterator tetra_it = tetras.begin();
       tetra_it != tetras.end(); ++tetra_it) {
    TetraBuilding* tetra = *tetra_it;
    if (tetra->inCylinder(pos, radius, height)) {
      if (location != NULL) {
        *location = tetra;
      }
      return IN_TETRA;
    }
  }
  for (std::vector<Teleporter*>::iterator tele_it = teleporters.begin();
       tele_it != teleporters.end(); ++tele_it) {
    Teleporter* tele = *tele_it;
    if (tele->inCylinder(pos, radius, height)) {
      if (location != NULL) {
        *location = tele;
      }
      return IN_TELEPORTER;
    }
  }

  if (location != NULL) {
    *location = (Obstacle *)NULL;
  }

  return NOT_IN_BUILDING;
}


InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
				             const float* pos, float radius,
                                             float height)
{
  if (height < Epsilon) {
    height = Epsilon;
  }

  *location = NULL;

  // check everything but walls
  const ObsList* olist = collisionManager.cylinderTest (pos, radius, height);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inCylinder(pos, radius, height)) {
      *location = obs;
      break;
    }
  }

  return classifyHit (*location);
}

InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
				             float x, float y, float z, float radius,
                                             float height)
{
  const float pos[3] = {x, y, z};
  return cylinderInBuilding (location, pos, radius, height);
}

InBuildingType WorldInfo::boxInBuilding(const Obstacle **location,
				        const float* pos, float angle,
				        float width, float breadth, float height)
{
  if (height < Epsilon) {
    height = Epsilon;
  }

  *location = NULL;

  // check everything but walls
  const ObsList* olist =
    collisionManager.boxTest (pos, angle, width, breadth, height);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inBox(pos, angle, width, breadth, height)) {
      *location = obs;
      break;
    }
  }

  return classifyHit (*location);
}

InBuildingType WorldInfo::classifyHit (const Obstacle* obstacle)
{
  if (obstacle == NULL) {
    return NOT_IN_BUILDING;
  }
  else if (obstacle->getType() == BoxBuilding::getClassName()) {
    if (obstacle->isDriveThrough()) {
      return IN_BOX_DRIVETHROUGH;
    }
    else {
      return IN_BOX_NOTDRIVETHROUGH;
    }
  }
  else if (obstacle->getType() == PyramidBuilding::getClassName()) {
    return IN_PYRAMID;
  }
  else if (obstacle->getType() == TetraBuilding::getClassName()) {
    return IN_TETRA;
  }
  else if (obstacle->getType() == MeshFace::getClassName()) {
    return IN_MESHFACE;
  }
  else if (obstacle->getType() == BaseBuilding::getClassName()) {
    return IN_BASE;
  }
  else if (obstacle->getType() == Teleporter::getClassName()) {
    return IN_TELEPORTER;
  }
  else {
    // FIXME - choke here?
    printf ("*** Unknown obstacle type in WorldInfo::boxInBuilding()\n");
    return IN_BASE;
  }
}

bool WorldInfo::getZonePoint(const std::string &qualifier, float *pt)
{
  const Obstacle* loc;
  InBuildingType type;

  if (!entryZones.getZonePoint(qualifier, pt))
    return false;

  type = cylinderInBuilding(&loc, pt[0], pt[1], 0.0f, 1.0f, pt[2]);
  if (type == NOT_IN_BUILDING)
    pt[2] = 0.0f;
  else
    pt[2] = loc->getPosition()[2] + loc->getSize()[2];
  return true;
}

bool WorldInfo::getSafetyPoint(const std::string &qualifier, const float *pos, float *pt)
{
  const Obstacle *loc;
  InBuildingType type;

  if (!entryZones.getSafetyPoint(qualifier, pos, pt))
    return false;

  type = cylinderInBuilding(&loc, pt[0], pt[1], 0.0f, 1.0f, pt[2]);
  if (type == NOT_IN_BUILDING)
    pt[2] = 0.0f;
  else
    pt[2] = loc->getPosition()[2] + loc->getSize()[2];
  return true;
}

void WorldInfo::finishWorld()
{
  entryZones.calculateQualifierLists();
  loadCollisionManager ();
}

int WorldInfo::packDatabase(const BasesList* baseList)
{
  // make default water material. we wait to make the default material
  // to avoid messing up any user indexing. this has to be done before
  // the texture matrices and materials are packed.
  if ((waterLevel >= 0.0f) && (waterMatRef == NULL)) {
    makeWaterMaterial();
  }

  std::vector<TetraBuilding*>::iterator tetra_it;
  std::vector<MeshObstacle*>::iterator mesh_it;
  int numBases = 0;
  BasesList::const_iterator base_it;
  if (baseList != NULL) {
    for (base_it = baseList->begin(); base_it != baseList->end(); ++base_it) {
      numBases += base_it->second.size();
    }
  }

  databaseSize =
    (2 + 2 + WorldCodeBaseSize) * numBases +
    (2 + 2 + WorldCodeWallSize) * walls.size() +
    (2 + 2 + WorldCodeBoxSize) * boxes.size() +
    (2 + 2 + WorldCodePyramidSize) * pyramids.size() +
    (2 + 2 + WorldCodeArcSize) * arcs.size() +
    (2 + 2 + WorldCodeConeSize) * cones.size() +
    (2 + 2 + WorldCodeSphereSize) * spheres.size() +
    (2 + 2 + WorldCodeTeleporterSize) * teleporters.size() +
    (2 + 2 + WorldCodeLinkSize) * 2 * teleporters.size() +
    worldWeapons.packSize() + entryZones.packSize() +
    DYNCOLORMGR.packSize() + TEXMATRIXMGR.packSize() +
    MATERIALMGR.packSize() + PHYDRVMGR.packSize();
  // add water level size
  databaseSize += sizeof(float);
  if (waterLevel >= 0.0f) {
    databaseSize += sizeof(int);
  }
  // tetra sizes are variable
  for (tetra_it = tetras.begin(); tetra_it != tetras.end(); ++tetra_it) {
    TetraBuilding* tetra = *tetra_it;
    databaseSize = databaseSize + (2 + 2);
    databaseSize = databaseSize + tetra->packSize();
  }
  // meshes have variable sizes
  for (mesh_it = meshes.begin(); mesh_it != meshes.end(); ++mesh_it) {
    MeshObstacle &mesh = (**mesh_it);
    if (!mesh.getIsLocal()) {
      databaseSize += (2 + 2) + mesh.packSize();
    }
  }

  database = new char[databaseSize];
  void *databasePtr = database;

  unsigned char	bitMask;

  // add dynamic colors
  databasePtr = DYNCOLORMGR.pack(databasePtr);

  // add texture matrices
  databasePtr = TEXMATRIXMGR.pack(databasePtr);

  // add materials
  databasePtr = MATERIALMGR.pack(databasePtr);

  // add physics drivers
  databasePtr = PHYDRVMGR.pack(databasePtr);

  // add water level
  databasePtr = nboPackFloat(databasePtr, waterLevel);
  if (waterLevel >= 0.0f) {
    int matindex = MATERIALMGR.getIndex(waterMatRef);
    databasePtr = nboPackInt(databasePtr, matindex);
  }

  // add meshes
  for (mesh_it = meshes.begin(); mesh_it != meshes.end(); ++mesh_it) {
    MeshObstacle &mesh = **mesh_it;
    if (!mesh.getIsLocal()) {
      databasePtr = nboPackUShort(databasePtr, WorldCodeMeshSize); // dummy
      databasePtr = nboPackUShort(databasePtr, WorldCodeMesh);
      databasePtr = mesh.pack(databasePtr);
      if (debugLevel > 3) {
        mesh.print(std::cout, 3);
      }
    }
  }

  // add arcs
  for (std::vector<ArcObstacle*>::iterator arc_it = arcs.begin();
       arc_it != arcs.end(); ++arc_it) {
    ArcObstacle& arc = **arc_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeArcSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeArc);
    databasePtr = arc.pack(databasePtr);
  }

  // add cones
  for (std::vector<ConeObstacle*>::iterator cone_it = cones.begin();
       cone_it != cones.end(); ++cone_it) {
    ConeObstacle& cone = **cone_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeConeSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeCone);
    databasePtr = cone.pack(databasePtr);
  }

  // add spheres
  for (std::vector<SphereObstacle*>::iterator sphere_it = spheres.begin();
       sphere_it != spheres.end(); ++sphere_it) {
    SphereObstacle& sphere = **sphere_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeSphereSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeSphere);
    databasePtr = sphere.pack(databasePtr);
  }

  // add tetras
  for (tetra_it = tetras.begin(); tetra_it != tetras.end(); ++tetra_it) {
    TetraBuilding& tetra = **tetra_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeTetraSize); // dummy
    databasePtr = nboPackUShort(databasePtr, WorldCodeTetra);
    databasePtr = tetra.pack(databasePtr);
  }

  // add bases
  if (baseList != NULL) {
    for (base_it = baseList->begin(); base_it != baseList->end(); ++base_it) {
      databasePtr = base_it->second.pack(databasePtr);
    }
  }

  // add walls
  for (std::vector<WallObstacle*>::iterator wall_it = walls.begin();
       wall_it != walls.end(); ++wall_it) {
    WallObstacle& wall = **wall_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeWallSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeWall);
    databasePtr = nboPackVector(databasePtr, wall.getPosition());
    databasePtr = nboPackFloat(databasePtr, wall.getRotation());
    databasePtr = nboPackFloat(databasePtr, wall.getSize()[1]);
    // walls have no depth
    databasePtr = nboPackFloat(databasePtr, wall.getSize()[2]);
  }

  // add boxes
  for (std::vector<BoxBuilding*>::iterator box_it = boxes.begin();
       box_it != boxes.end(); ++box_it) {
    BoxBuilding& box = **box_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeBoxSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeBox);
    databasePtr = nboPackVector(databasePtr, box.getPosition());
    databasePtr = nboPackFloat(databasePtr, box.getRotation());
    databasePtr = nboPackVector(databasePtr, box.getSize());
    bitMask = 0;
    if (box.isDriveThrough())
      bitMask |= _DRIVE_THRU;
    if (box.isShootThrough())
      bitMask |= _SHOOT_THRU;
    databasePtr = nboPackUByte(databasePtr, bitMask);
  }

  // add pyramids
  for (std::vector<PyramidBuilding*>::iterator pyr_it = pyramids.begin();
       pyr_it != pyramids.end(); ++pyr_it) {
    PyramidBuilding& pyr = **pyr_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramidSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramid);
    databasePtr = nboPackVector(databasePtr, pyr.getPosition());
    databasePtr = nboPackFloat(databasePtr, pyr.getRotation());
    databasePtr = nboPackVector(databasePtr, pyr.getSize());
    bitMask = 0;
    if (pyr.isDriveThrough())
      bitMask |= _DRIVE_THRU;
    if (pyr.isShootThrough())
      bitMask |= _SHOOT_THRU;
    if (pyr.getZFlip())
      bitMask |= _FLIP_Z;
    databasePtr = nboPackUByte(databasePtr, bitMask);
  }

  // add teleporters
  int i = 0;
  for (std::vector<Teleporter*>::iterator tele_it = teleporters.begin();
       tele_it != teleporters.end(); ++tele_it, i++) {
    Teleporter& tele = **tele_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporterSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporter);
    databasePtr = nboPackVector(databasePtr, tele.getPosition());
    databasePtr = nboPackFloat(databasePtr, tele.getRotation());
    databasePtr = nboPackVector(databasePtr, tele.getSize());
    databasePtr = nboPackFloat(databasePtr, tele.getBorder());
	databasePtr = nboPackUByte(databasePtr, tele.isHorizontal());
    bitMask = 0;
    if (tele.isDriveThrough())
      bitMask |= _DRIVE_THRU;
    if (tele.isShootThrough())
      bitMask |= _SHOOT_THRU;
    databasePtr = nboPackUByte(databasePtr, bitMask);
    // and each link
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2));
    databasePtr = nboPackUShort(databasePtr, uint16_t(teleportTargets[i * 2]));
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2 + 1));
    databasePtr = nboPackUShort(databasePtr, uint16_t(teleportTargets[i * 2 + 1]));
  }

  databasePtr = worldWeapons.pack (databasePtr);
  databasePtr = entryZones.pack (databasePtr);

  // compress the map database
  uLongf gzDBlen = databaseSize + (databaseSize/512) + 12;
  char* gzDB = new char[gzDBlen];
  int code = compress2 ((Bytef*)gzDB, &gzDBlen,
                        (Bytef*)database, databaseSize, 9);
  if (code != Z_OK) {
    printf ("Could not create compressed world database: %i\n", code);
    exit (1);
  }

  // switch to the compressed map database
  uncompressedSize = databaseSize;
  databaseSize = gzDBlen;
  char *oldDB = database;
  database = gzDB;
  delete[] oldDB;

  DEBUG1 ("Map size: uncompressed = %i, compressed = %i\n",
           uncompressedSize, databaseSize);

  return 1;
}

void *WorldInfo::getDatabase() const
{
  return database;
}

int WorldInfo::getDatabaseSize() const
{
  return databaseSize;
}

int WorldInfo::getUncompressedSize() const
{
  return uncompressedSize;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
