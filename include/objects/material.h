#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "../DataEntry.h"
#include "../render/RGBA.h"
#include "../model/BZWParser.h"
#include "../model/Model.h"

#include <string>
#include <vector>

#include <osg/Material>
#include <osg/Texture2D>
#include <osg/StateSet>

using namespace std;

class dynamicColor;
class texturematrix;

#define IS_VALID_COLOR( c ) (c.x() >= 0.0 && c.y() >= 0.0 && c.z() >= 0.0 && c.w() >= 0.0)

class material : public DataEntry, public osg::StateSet {

public:
	// default constructor
	material();
	
	// constructor with data
	material(string& data);
	
	static DataEntry* init() { return new material(); }
	static DataEntry* init(string& data) { return new material(data); }
	
	// getter
	string get(void);
	
	// setter
	int update(string& data);
	
	// tostring
	string toString(void);
	
	// binary getters and setters
	string getName() { return name; }
	dynamicColor* getDynamicColor() { return dynCol; }
	texturematrix* getTextureMatrix() { return textureMatrix; }
	
	vector< osg::ref_ptr< material > >& getMaterials() { return materials; }
	vector< osg::ref_ptr< osg::Texture2D > >& getTextures() { return textures; }
	
	const osg::Vec4& getAmbient() { return getCurrentMaterial()->getAmbient( osg::Material::FRONT ); }
	const osg::Vec4& getDiffuse() { return getCurrentMaterial()->getDiffuse(osg::Material::FRONT); }
	const osg::Vec4& getSpecular() { return getCurrentMaterial()->getSpecular(osg::Material::FRONT); }
	const osg::Vec4& getEmissive() { return getCurrentMaterial()->getEmission(osg::Material::FRONT); }
	float getShininess() { return getCurrentMaterial()->getShininess(osg::Material::FRONT); }
	
	float getAlphaThreshold() { return alphaThreshold; }
	bool usesTextures() { return !noTextures; }
	bool usesTexColor() { return !noTexColor; }
	bool usesSphereMap() { return spheremap; }
	bool usesShadows() { return !noShadow; }
	bool usesCulling() { return !noCulling; }
	bool usesRadar() { return !noRadar; }
	bool usesGroupAlpha() { return groupAlpha; }
	bool usesOccluder() { return occluder; }
	
	void setName( const string& name ) { this->name = name; }
	void setDynamicColor( dynamicColor* dynCol ) { this->dynCol = dynCol; }
	void setTextureMatrix( texturematrix* texmat ) { this->textureMatrix = texmat; }
	
	void setMaterials( vector< osg::ref_ptr< material > >& mats ) {
		this->materials = materials;
		computeFinalMaterial();
	}
	
	void setTextures( const vector< osg::ref_ptr< osg::Texture2D > >& textures ) {
		this->textures = textures;
		computeFinalTexture();
	}
	
	void setAmbient( const osg::Vec4& rgba ) { getCurrentMaterial()->setAmbient( osg::Material::FRONT, rgba ); }
	void setDiffuse( const osg::Vec4& rgba ) { getCurrentMaterial()->setDiffuse( osg::Material::FRONT, rgba ); }
	void setSpecular( const osg::Vec4& rgba ) { getCurrentMaterial()->setSpecular( osg::Material::FRONT, rgba ); }
	void setEmissive( const osg::Vec4& rgba ) { getCurrentMaterial()->setEmission( osg::Material::FRONT, rgba ); }
	// setters with RGBA values
	void setAmbient( const RGBA& rgba ) { this->setAmbient( osg::Vec4( rgba.x(), rgba.y(), rgba.z(), rgba.w() ) ); }
	void setDiffuse( const RGBA& rgba ) { this->setDiffuse( osg::Vec4( rgba.x(), rgba.y(), rgba.z(), rgba.w() ) ); }
	void setSpecular( const RGBA& rgba ) { this->setSpecular( osg::Vec4( rgba.x(), rgba.y(), rgba.z(), rgba.w() ) ); }
	void setEmissive( const RGBA& rgba ) { this->setEmissive( osg::Vec4( rgba.x(), rgba.y(), rgba.z(), rgba.w() ) ); }
	
	void setShininess( float shininess ) { getCurrentMaterial()->setShininess( osg::Material::FRONT, shininess );  }
	
	void setAlphaThreshold( float alphaThreshold ) { this->alphaThreshold = alphaThreshold; }
	void useTextures( bool value ) { this->noTextures = !value; }
	void useTexColor( bool value ) { this->noTexColor = !value; }
	void useSphereMap( bool value ) { this->spheremap = value; }
	void useShadows( bool value ) { this->noShadow = !value; }
	void useCulling( bool value ) { this->noCulling = !value; }
	void useRadar( bool value ) { this->noRadar = !value; }
	void useGroupAlpha( bool value ) { this->groupAlpha = value; }
	void useOccluder( bool value ) { this->occluder = value; }
	
	// use this to compute the osg stateset to apply
	// this entails merging parts of other materials
	static material* computeFinalMaterial( vector< osg::ref_ptr< material > >& materialList );
	
	// get the current material
	osg::Material* getCurrentMaterial();
	
	// get the current texture
	osg::Texture2D* getCurrentTexture();
	
private:
	string name, color;
	dynamicColor* dynCol;
	texturematrix* textureMatrix;
	
	vector< osg::ref_ptr< material > > materials;		// the various material references
	vector< osg::ref_ptr< osg::Texture2D > > textures;	// the various textures from "texture" and "addTexture"
	
	bool noTextures, noTexColor, noTexAlpha, spheremap, noShadow, noCulling, noSorting, noRadar, noLighting, groupAlpha, occluder;
	float alphaThreshold;
	
	// compute the final OSG texture
	void computeFinalTexture();
	
	// compute the final OSG material
	void computeFinalMaterial();
};

#endif /*MATERIAL_H_*/
