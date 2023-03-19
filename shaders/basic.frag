#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

in vec4 fragPosEye;
in vec4 fragPosLightSpace;

out vec4 fColor;

struct Light
{
	vec3 position;
	float constant;
	float linear;
	float quadratic;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

//matrices
uniform Light pointLight;
uniform Light fixedLight1;
uniform Light fixedLight2;

uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform int fogEnable;
uniform int directionalLightAndShadowsEnable;

uniform vec3 viewPos;


float getFog(float density)
{
	float ret = exp(-density  * length(fragPosEye));
	return clamp(ret, 0.f, 1.f);
}

//components
vec3 ambient;
float ambientStrength = 0.02f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.05f;

void computeDirectionalLight()
{
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));
    vec3 viewDir = normalize(- fPosEye.xyz);

    ambient = ambientStrength * lightColor;
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 16);
    specular = specularStrength * specCoeff * lightColor;
}

vec3 getPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	
	vec3 lightDirection = normalize(light.position - fragPos);
	
	float diffCoeff = max(dot(normal, lightDirection), 0.f);
	
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specCoeff = pow(max(dot(viewDir, reflectionDirection), 0.f), 32.f);
	
	float distance = length(light.position - fragPos);
	float att = 1.f / (light.constant + light.linear * distance + light.quadratic * pow(distance, 2.f));
	
	vec3 ambient = light.ambient * vec3(texture(diffuseTexture, fTexCoords));
	vec3 diffuse = light.diffuse * diffCoeff * vec3(texture(diffuseTexture, fTexCoords));
	vec3 specular = light.specular * specCoeff * lightColor;
	
	vec3 ret = (ambient + specular + diffuse) * att;
	return ret;
	
}

float computeShadow()
{	
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
 
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    float currentDepth = normalizedCoords.z;
    float bias = 0.005f;
	float shadow = 0.0;
	
	
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, normalizedCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	
	shadow /= 9.0;
    return shadow;	
}

void main() 
{
	vec3 fogColor = vec3(0.4f, 0.4f, 0.4f);
	vec3 viewDir = normalize(viewPos - fPosition);
	
    computeDirectionalLight();
	float shadow = computeShadow();
	
	vec3 color;
	if (directionalLightAndShadowsEnable == 1)
		color = min((ambient +  (1.0f - shadow) * diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow) * specular, 1.0f);
	else
		color = texture(diffuseTexture, fTexCoords).rgb;

	color += getPointLight(pointLight, normalize(fNormal), fPosition, viewDir);
	color += getPointLight(fixedLight1, normalize(fNormal), fPosition, viewDir);
	color += getPointLight(fixedLight2, normalize(fNormal), fPosition, viewDir);
	
    fColor = vec4(color, 1.0f);
	
	if (fogEnable == 1)
		fColor = mix(vec4(fogColor, 1.f), fColor, getFog(0.008f));
}
