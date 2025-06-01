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




void main()
{
    idx = uint(gl_InstanceIndex);
    RenderObject ro = render_objects[idx];

    const uint idx = uint(gl_InstanceIndex);
	const uint vdx = uint(gl_VertexIndex);
    
    //const uint gdi = uint(gl_BaseVertex);
    //const uint gdi = uint(gl_BaseInstance);

	//select proper vert from instanceindex and vertex id
	vec4 posCoord = positions[vdx];
    vec4 normCoord = normals[vdx];

    fragPos = vec3(model_matricies[ro.matrix] * posCoord);
    //interpolatedNormal = normCoord.xyz;

    interpolatedNormal = mat3(transpose(inverse(model_matricies[ro.matrix]))) * normCoord.xyz;

    interpolatedTexCoord = texcoords[vdx].xy;

	gl_Position =  cameras[ro.camera].proj * cameras[ro.camera].view * model_matricies[ro.matrix] * posCoord;

}

