#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../common.glsl"

layout(location = 0) out vec4 outColor;
layout(location = 3) flat in uint idx;
layout(location = 4) in vec3 interpolatedNormal;
layout(location = 5) in vec2 interpolatedTexCoord;
layout(location = 6) in vec3 viewDir;
layout(location = 7) in vec3 lightDir;
layout(location = 8) in float lightDistance2;
layout(location = 9) in vec3 fragPos;
layout(location = 10) flat in uint grd;

void main()
{
    Light light = lights[0];
    Camera camera = cameras[0];

    vec3 norm = normalize(interpolatedNormal);
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    vec3 viewDir = normalize(camera.position.xyz - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular.w * spec * light.diffuse.xyz; 
    
    float diff = max(dot(norm, light.direction.xyz), 0.0);
    vec3 diffuse = diff * light.diffuse.xyz;

    vec3 result = (((light.ambient.xyz*vec3(1.0,0.33,0.1)) + diffuse + specular) * vec3(0.33,0.33,0.1));

    vec2 tc = interpolatedTexCoord.xy;
        
    tc.x /= textureWidth;
    tc.y /= textureHeight;
    tc.x *= textureTileWidth;
    tc.y *= textureTileHeight;

    tc.x += mod(grd,textureNumTilesX) * (tileCoordWidth);
    tc.y += floor(grd / textureNumTilesX) / textureNumTilesY;

    vec4 sam = texture(texsamp,tc);


    outColor = sam * sam.a;
}
