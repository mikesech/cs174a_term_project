#version 150 core

precision mediump float;

//Constants: samplers
uniform sampler2D diffuseMap;
uniform sampler2D NormalMap;

//Normal Map "Depth" Scaling
uniform float normalMapDepth;
uniform bool hasTexture;
uniform bool hasFog;

//Constants: material properties
uniform vec4 diffuseColor;
uniform float shininess;
uniform float alpha;

//Constants:
uniform vec2 uvScale;
uniform vec2 uvOffset; //what is this?
uniform bool infiniteDepthOnTranslucent;

//Constants: lighting
uniform vec4 ambientColor;
uniform vec4 lightPos[10];
uniform float lightFalloff[10];
uniform vec4 lightColor[10];
uniform float lightBrightness[10];

//Constants: Camera and moodel Properties
uniform mat4 vTransform;
uniform mat4 camTransform;
uniform vec4 camPos; //camera position in worldspace

//Inputs
in vec4 fPosition_worldspace;
in vec4 fNormal_worldspace;
in vec2 fUV;
in vec4 o_vertexNormal_modelspace;
in vec4 o_vertexTangent_modelspace;
in vec4 o_vertexBitangent_modelspace;
in vec3 baryCoord;



//Output: pixel color output
out vec4 fColor;

vec4 hueToFullColor(float hue) {
	float h = mod(hue, 360.0f);
	if (h < 0.0) h += 360.0f; // Ensure hue is positive
	float x = 1.0 - abs(mod(h / 60.0f, 2.0) - .0);
	float r = 0.0, g = 0.0, b = 0.0;

	if      (  0.0 <= h && h <  60.0) { r = 1.0; g = x; b = 0.0; }
	else if ( 60.0 <= h && h < 120.0) { r = x; g = 1.0; b = 0.0; }
	else if (120.0 <= h && h < 180.0) { r = 0.0; g = 1.0; b = x; }
	else if (180.0 <= h && h < 240.0) { r = 0.0; g = x; b = 1.0; }
	else if (240.0 <= h && h < 300.0) { r = x; g = 0.0; b = 1.0; }
	else if (300.0 <= h && h < 360.0) { r = 1.0; g = 0.0; b = x; }

	return vec4(r, g, b, 1.0);
}

void main(){
	gl_FragDepth = gl_FragCoord.z;
	float edgeDist = min(min(baryCoord.x, baryCoord.y), baryCoord.z);
	float pixelDist = edgeDist / (fwidth(edgeDist));  // approximate distance in pixels
	
	//if (pixelDist <= 1.0) {
	//	fColor = vec4(1.0, 1.0, 1.0, 1.0);
	//}
	//float edgeAlpha = 1.0 - smoothstep(0.0, 1.0, pixelDist); // Smooth transition: fade in the wire near edges (1–2 px)
	// step(2.0, pixelDist); for hard cutoff
    // edgeAlpha = pow(edgeAlpha, 1.5); // Optional: sharpen by clamping or boosting contrast
    //fColor = mix(vec4(0.0, 0.0, 0.0, 0.2), vec4(1.0), edgeAlpha);
	//return;

	mat4 TBN = transpose(mat4(o_vertexTangent_modelspace,
							  o_vertexBitangent_modelspace,
							  o_vertexNormal_modelspace,
							  vec4(0,0,0,0) )); //from worldspace to tbn space 4x4 since every normal is 4x4.

	vec4 diffusePass  = vec4(0,0,0,1);
	vec4 specularPass = vec4(0,0,0,0);

	vec4 viewVec = camPos-fPosition_worldspace;
	viewVec.w    = 0.0;
	vec4 viewVec_tbn      = normalize(TBN * (normalize(viewVec))); //eye direction in tbn space
	/////////
	//viewVec_tbn      =  (normalize(viewVec)); //in tbn space

	// some texture color
	vec4 texColor = texture(diffuseMap, fUV); //fUV has offset and stuff already applied.

	// Local normal, in tangent space. V tex coordinate is inverted because normal map is in TGA (not in DDS)
	//vec4 TextureNormal_tangentspace = vec4(normalize(texture( NormalMap, vec2(fUV.x,-fUV.y) ).rgb * 2.0 - 1.0), 0);
	vec4 TextureNormal_tangentspace = vec4(normalize(texture( NormalMap, vec2(fUV.x, fUV.y) ).rgb * 2.0 - 1.0), 0);

	vec4 n = normalize(normalMapDepth*TextureNormal_tangentspace+(1.0-normalMapDepth)*vec4(0,0,1,0));
	/////////
	//n = fNormal_worldspace;
	//n=TextureNormal_tangentspace;

	//n=normalize(vec4(0,0,1,0)*(1.0-max(normalMapDepth,0.0)));

	if(dot(normalize(viewVec),fNormal_worldspace)<0.0&&alpha!=1.0){
		fColor.w=0.0;
		return;
	}


	for(int i=0; i<10; i++)
	{
		vec4 lightVec = lightPos[i] - fPosition_worldspace;
		lightVec.w    = 0.0;
		vec4 lightVecNorm_tbn = normalize(TBN * normalize(lightVec)); //in tbn space

		////////
		//lightVecNorm_tbn = (normalize(lightVec)); //in tbn space

		//Diffuse Pass
		float diffuseMult = dot(n, lightVecNorm_tbn);
		
		diffusePass  += max(lightColor[i]* diffuseMult * (texColor + diffuseColor) * (1.0/pow(dot(lightVec, lightVec), lightFalloff[i]/2.0)) * lightBrightness[i],vec4(0,0,0,1));
		//diffusePass+=vec4(1,1,1,1)*diffuseMult;
		diffusePass   = clamp(diffusePass, 0.0, 1.0);
		

		//Specular Pass
		float specularMult;
		if(shininess <= 0.0){
			specularMult = 0.0;
		}else{
			specularMult = pow(max(dot(viewVec_tbn, normalize(-reflect(lightVecNorm_tbn, n))) ,0.0), shininess);
		}
		specularPass += lightColor[i] * specularMult * diffuseMult * lightBrightness[i];
		specularPass  = clamp(specularPass,0.0,1.0);
	}

	diffusePass.w = alpha;
	if(hasTexture){
		diffusePass.w=alpha*texColor.w;
	}




	fColor = vec4(ambientColor.xyz,0)+diffusePass+specularPass;
	if (infiniteDepthOnTranslucent && fColor.w >= 0.75) {
		fColor.w = 1.0;
	}
	if (infiniteDepthOnTranslucent && fColor.w < 1.0) {
		gl_FragDepth = 1000000000000000000000000.0;
	}
	
	if(hasFog){
		vec4 fogColor=vec4(.35,.4,.375,0);
		float fogMult=min(pow(dot(viewVec,viewVec),.8)*.00025,1.0);

		fColor=fColor+fogColor*fogMult;
	}
}
