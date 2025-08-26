#include "WorldEntity.h"
#include "Engine.h"

unsigned long long WorldEntity::globalOrderingCount = 0;

WorldEntity::WorldEntity(const WorldEntity* parent)
{
	_parent = parent;
	if (_parent != NULL)
		_parent->_isParent = true;
	_position = vec3(0, 0, 0);
	_rotation = vec3(0, 0, 0);
	_scale    = vec3(1, 1, 1);
	_totalOrderingCount = 0;
}

void WorldEntity::invalidateCache()
{
	if(_isParent) {
		++globalOrderingCount;
	} else {
		_totalOrderingCount = globalOrderingCount - 1;
	}
}

//Translation
void WorldEntity::translate(vec3 v)
{
	_position += v;
	invalidateCache();
}

void WorldEntity::translate(float x, float y, float z)
{
	_position += vec3(x, y, z);
	invalidateCache();
}

void WorldEntity::setTranslate(float x, float y, float z)
{
	_position = vec3(x, y, z);	
	invalidateCache();
}
void WorldEntity::setTranslate(vec3 v){
	_position=v;
	invalidateCache();
}

void WorldEntity::setTranslateX(float x)
{
	_position.x=x;
	invalidateCache();
}
void WorldEntity::setTranslateY(float y)
{
	_position.y=y;
	invalidateCache();
}
void WorldEntity::setTranslateZ(float z)
{
	_position.z = z;
	invalidateCache();
}
vec3 WorldEntity::getTranslate() const
{
	if(_totalOrderingCount != globalOrderingCount) {
		_totalOrderingCount = globalOrderingCount;

		if (_parent == NULL){
			_worldPosition = _position;
		}else{
			vec4 temp = _parent->getTransformationMatrix()*vec4(_position,1.0);
			_worldPosition=vec3(temp.x,temp.y,temp.z);
		}
	}
	return _worldPosition;
}



//Rotation
void WorldEntity::rotate(float x, float y, float z)
{
	_rotation += vec3(x, y, z);
	invalidateCache();
}
void WorldEntity::setRotate(float x, float y, float z)
{
	_rotation = vec3(x, y, z);
	invalidateCache();
}
vec3 WorldEntity::getRotate() const
{
	vec3 r;
	if (_parent == NULL)
		r = _rotation;
	else
		r = _parent->getRotate() + _rotation;
	
	return r;
}


//Scale
void WorldEntity::scale(float x, float y, float z)
{
	_scale.x*=x;
	_scale.y*=y;
	_scale.z*=z;
	invalidateCache();
}
void WorldEntity::setScale(float x, float y, float z)
{
	_scale = vec3(x, y, z);
	invalidateCache();
}
vec3 WorldEntity::getScale() const
{
	vec3 s;
	if (_parent == NULL)
		s = _scale;
	else
		s = vec3(_parent->getScale().x*_scale.x,_parent->getScale().y*_scale.y,_parent->getScale().z*_scale.z);
	
	return s;
}


//TransformationMatrix
mat4 WorldEntity::getTransformationMatrix() const
{

	mat4 scaleMat=mat4();
	scaleMat[0][0]=_scale.x;
	scaleMat[1][1]=_scale.y;
	scaleMat[2][2]=_scale.z;
	scaleMat[3][3]=1;

	mat4 m = Translate(_position.x, _position.y, _position.z) * RotateX(_rotation.x) * RotateY(_rotation.y) * RotateZ(_rotation.z) * scaleMat;

	if(_parent == NULL)
		return m;
	else
		return _parent->getTransformationMatrix() * m;
}

mat4 WorldEntity::getCameraTransformationMatrix() const{
	mat4 m = RotateX(-_rotation.x) * RotateY(-_rotation.y) * RotateZ(-_rotation.z)*Translate(-_position.x, -_position.y, -_position.z) ;

	if(_parent == NULL)
		return m;
	else
		return  m * _parent->getCameraTransformationMatrix();
}