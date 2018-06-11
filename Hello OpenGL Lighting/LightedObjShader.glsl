#version 330 core
out vec4 FragColor;

in vec3 aNormal;
in vec2 pTextureCoordinate;
in vec3 PixelPos;
in mat4x4 aViewMatrix;
uniform vec3 objectColor;
//Directional Light
uniform vec3 aLightDir;
//uniform vec3 LightPos;
uniform sampler2D TextureBack;
uniform sampler2D TextureFront;
uniform sampler2D SpecMap;
uniform mat3x3 NormalMatrix;
uniform mat3x3 VectorMatrix;
uniform vec3 diffuseColor,ambientColor,specularColor;
uniform float shininess;

void main()
{
	//物体的基本颜色(贴图)
	vec4 px=texture(TextureFront,pTextureCoordinate);
	float MixAlpha=1-(px.x+px.y+px.z)/3.f;
	MixAlpha=pow(MixAlpha*50,10);
	vec4 FragColorx=mix(texture(TextureBack,pTextureCoordinate),texture(TextureFront,pTextureCoordinate),clamp(MixAlpha*50,0,1.f));
	vec2 xtmpC=pTextureCoordinate-vec2(0.5f,0.5f);
	if(abs(xtmpC.x)>0.4||abs(xtmpC.y)>0.4)
		FragColorx=texture(TextureBack,pTextureCoordinate);
	//~~
	
	vec3 LightDir=-VectorMatrix*aLightDir;
	vec3 Normal=normalize(NormalMatrix*aNormal);
	//vec3 LightDir=normalize(LightPos-PixelPos);
	vec3 ViewDir = normalize(vec3(0,0,0) - PixelPos);
	vec3 ReflectDir = reflect(-LightDir, Normal);
	float SpecularStrength=pow(clamp(dot(ReflectDir,ViewDir),0,1),shininess);
	float DiffStrength=clamp(dot(Normal,LightDir),0,1);
	vec3 diffuse=DiffStrength * FragColorx.xyz;
	vec3 Specular=SpecularStrength*specularColor*texture(SpecMap,pTextureCoordinate).xyz;
	float ambientStrength=0.1;
	float Emissive=clamp(FragColorx.z-(FragColorx.x+FragColorx.y)/2.f,0,1);
    FragColor = vec4(ambientStrength*ambientColor*FragColorx.xyz+diffuse+Specular+Emissive*FragColorx.xyz, 1.0);
}