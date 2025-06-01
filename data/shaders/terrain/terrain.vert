#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../common.glsl"

layout(location = 3) flat out uint idx;

layout(location = 4) out vec3 interpolatedNormal;
layout(location = 5) out vec2 interpolatedTexCoord;
layout(location = 6) out vec3 viewDir;
layout(location = 7) out vec3 lightDir;
layout(location = 8) out float lightDistance2;
layout(location = 9) out vec3 fragPos;

mat4 tr(vec4 delta)
{
    mat4 m = mat4(1.0);
    m[0][0] = 1;
    m[1][1] = 1;
    m[2][2] = 1;
    m[3] = delta;
    return m;
}

void main()
{

	const float terrainWidth = 256;
	const float terrainHeight = 256;
	const uint numVerts = 36;
	const uint numIndicies = 36;

	const uint idx = uint(gl_InstanceIndex);
	const uint vdx = uint(gl_VertexIndex);

	const uint tile = tiles[idx];
	const uint alt = alts[idx];

	//select proper vert from instanceindex and vertex id
	vec4 newVert = positions[(tile*numVerts)+vdx];
	newVert.x += (mod(idx, terrainWidth));
	newVert.z += terrainHeight - ( floor(idx / terrainWidth) );//255.0 - (floor(idx / terrainWidth) * boxDepth);
	newVert.y -= alt;
	const vec4 newNorm = normals[(tile*numVerts)+vdx];

    fragPos = newVert.xyz;
    //interpolatedNormal = normCoord.xyz;
    interpolatedNormal = mat3(transpose(inverse(mat4(1.0)))) * newNorm.xyz;
    interpolatedTexCoord = texcoords[vdx].xy;

	gl_Position = cameras[0].proj * cameras[0].view * newVert;

}
