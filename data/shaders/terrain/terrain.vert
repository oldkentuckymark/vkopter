#version 450

#extension GL_GOOGLE_include_directive : enable

layout(location = 3) flat out uint idx;

layout(location = 4) out vec3 interpolatedNormal;
layout(location = 5) out vec2 interpolatedTexCoord;
layout(location = 6) out vec3 viewDir;
layout(location = 7) out vec3 lightDir;
layout(location = 8) out float lightDistance2;
layout(location = 9) out vec3 fragPos;

layout(push_constant) uniform push_constants_t
{
        uint numLights;
        float maxLightDistance;
        float oldTime;
        float currentTime;
        uint terrainWidth;
        uint terrainHeight;
        uint reseved6;
        uint reseved7;
        uint reseved8;
        uint reseved9;
        uint reseved10;
        uint reseved11;
        uint reseved12;
        uint reseved13;
        uint reseved14;
        uint reseved15;
        uint reseved16;
        uint reseved17;
        uint reseved18;
        uint reseved19;
        uint reseved20;
        uint reseved21;
        uint reseved22;
        uint reseved23;
        uint reseved24;
        uint reseved25;
        uint reseved26;
        uint reseved27;
        uint reseved28;
        uint reseved29;
        uint reseved30;
        uint reseved31;
} PushConstants;

layout (set = 0, binding = 0) buffer readonly indicies_t
{
	uint indicies[];
};

layout (set = 0, binding = 1) buffer readonly positions_t
{
	vec4 positions[];
};

layout (set = 0, binding = 2) buffer readonly texcoords_t
{
	vec4 texcoords[];
};

layout (set = 0, binding = 3) buffer readonly normals_t
{
	vec4 normals[];
};

struct RenderObject
{
    uint mesh;
    uint material;
    uint camera;
    uint matrix;
};

layout (set = 0, binding = 4) buffer readonly render_objects_t
{
	RenderObject render_objects[];
};

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular; //specular.a = shininess
    vec4 emissive;
};

layout (set = 0, binding = 5) buffer readonly materials_t
{
	Material materials[];
};

struct Camera
{
    mat4 view;
    mat4 proj;
    vec4 position;
    vec4 target;
    vec4 up;
    vec4 velocity;
    float width;
    float height;
    float fov;
    float near;
    float far;
    uint p0;
    uint p1;
    uint p2;
    uint p3;
    uint p4;
    uint p5;
    uint p6;
    uint p7;
    uint p8;
    uint p9;
    uint p10;

};


layout (set = 0, binding = 6) buffer readonly cameras_t
{
	Camera cameras[];
};

layout (set = 0, binding = 7) buffer readonly model_matricies_t
{
	mat4 model_matricies[];
};  

struct Light
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    vec4 position;
    vec4 direction;

    //{constatt,linatt,quadatt,cutoffangle}
    vec4 attenuation;
}; 

layout (set = 0, binding = 8) buffer readonly lights_t
{
	Light lights[];
};  

layout (set = 0, binding = 9) buffer readonly tiles_t
{
	uint tiles[];
};  

layout (set = 0, binding = 10) buffer readonly alts_t
{
	uint alts[];
};  

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

	gl_Position = cameras[0].proj * cameras[0].view * newVert;

}
