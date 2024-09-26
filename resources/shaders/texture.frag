#version 330 core
in vec4 vertColor;
in vec2 vertTexCoord;
in vec3 vertNormal;
in vec3 fragPosition;
out vec4 outColor;

uniform sampler2D textureSampler;
uniform samplerCube shadowMap;
uniform float farPlane;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform float specularStrength;
uniform mat4 world;
uniform bool enableShadows;
uniform bool enableTextures;

const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

void main()
{
	float ambientStrength = 1.5f;
	vec3 ambient = ambientStrength * lightColor;
	
	vec3 lightDirection = normalize(fragPosition - lightPosition);
	float lightSourceDistanceFactor = pow(length(fragPosition - lightPosition), 1) / 8.0f;
	float diffuseFactor = max(dot(normalize(vertNormal), lightDirection), 0.0f) * 0.3f;
	vec3 diffuse = diffuseFactor * lightColor;
	
	vec3 viewDirection = normalize(fragPosition - cameraPosition);
	vec3 reflectDirection = reflect(-lightDirection, normalize(vertNormal));  
	float specularFactor = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
	vec3 specular = specularStrength * specularFactor * lightColor;
	
	vec3 FL = fragPosition - lightPosition;
    float closestDepth = texture(shadowMap, FL).r;
    closestDepth *= farPlane;
    float currentDepth = length(FL);
    float bias = 0.5; 
	float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;             
	
	vec3 finalFactor = enableShadows ? (ambient + (1.0f - shadow) * (ambient * 0.5f + diffuse) + specular) / lightSourceDistanceFactor :
	(ambient + diffuse + specular) / lightSourceDistanceFactor;
	vec3 finalColor = enableTextures ? texture(textureSampler, vertTexCoord).xyz : vertColor.xyz;
	vec3 final = finalFactor * finalColor;
    outColor = vec4(final, 1.0f);
} 