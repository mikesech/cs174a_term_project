#ifndef _DRAWABLEENTITY
#define _DRAWABLEENTITY

#include "WorldEntity.h"
#include "Engine.h"
class DrawableEntity:public WorldEntity{
private:
	char* _texName;
	vec3 _diffuseColor;
	float _alpha;
	char* _modelName;
	vec2 _uvOffset;
	vec2 _uvScale;
	float _shininess;

	bool _alphaFlag;

public:
	inline DrawableEntity(char* tn, char* mn, WorldEntity* parent=NULL)
		:WorldEntity(parent),_uvScale(vec2(1,1)),_uvOffset(vec2(0,0)),_shininess(0),_diffuseColor(vec3(0,1,0)),_alpha(1.0),_alphaFlag(false)
	{
		setTexture(tn);
		setModel(mn);

		if(tn!=NULL)CTextureManager::GetInstance()->GetTexture(tn);
		if(mn!=NULL)CRenderObjectManager::GetInstance()->GetRenderObject(mn)->activateObject(Globals::sProgram);
	}
	inline void draw()const{
		Globals::setTextureOffset(_uvOffset);
		Globals::setTextureScale(_uvScale);
		Globals::setShininess(_shininess);

		if(_texName!=NULL){
			Globals::setUseTexture(_texName);
			Globals::setDiffuseColor(vec3(0,0,0));
		}else {
			Globals::setUseTexture(0);
			Globals::setDiffuseColor(_diffuseColor);
		}
		Globals::setAlpha(_alpha);

		Globals::setModelTransMatrix(getTransformationMatrix());

		if(_modelName!=NULL)Globals::drawModel(_modelName);

	}
	inline void setShininess(float f){_shininess=f;}
	inline void setTexture(char* t){_texName=t;if(t=="")_texName=NULL;}
	inline void setAlpha(float a)
		{
			_alpha=a;
			if(a<1.0){setAlphaRequired(true);}
			else{setAlphaRequired(false);}
		}
	inline void setDiffuseColor(vec3 c){_diffuseColor=c;_texName=NULL;}
	inline void setDiffuseColor(vec4 c){setDiffuseColor(vec3(c.x,c.y,c.z));setAlpha(c.w);}
	inline void setDiffuseColor(float r, float g, float b, float a=1.0){setDiffuseColor(vec4(r,g,b,a));}
	inline void setModel(char* m){_modelName=m;if(m=="")_modelName=NULL;}
	inline void setUVOffset(float u, float v){_uvOffset=vec2(u,v);}
	inline void setUVScale(float u, float v){_uvScale=vec2(u,v);}

	inline void setAlphaRequired(bool t){_alphaFlag=t;}
	inline bool isAlphaRequired()const{return _alphaFlag;}

};

#endif