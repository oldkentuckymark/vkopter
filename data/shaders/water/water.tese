#version 450

layout( quads, equal_spacing, ccw ) in;

layout(location = 0) flat in uint idx_in[];
layout(location = 0) flat out uint idx;
layout(location = 1) in vec3 interpolatedNormal[];
layout(location = 1) out vec3 interpolatedNormal_out;
layout(location = 2) in vec2 interpolatedTexCoord[];
layout(location = 2) out vec2 interpolatedTexCoord_out;
layout(location = 3) in vec3 viewDir[];
layout(location = 3) out vec3 viewDir_out;
layout(location = 4) in vec3 lightDir[];
layout(location = 4) out vec3 lightDir_out;
layout(location = 5) in float lightDistance2[];
layout(location = 5) out float lightDistance2_out;
layout(location = 6) in vec3 fragPos[];
layout(location = 6) out vec3 fragPos_out;


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

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

    vec4 pos0 = gl_in[0].gl_Position;
    vec4 pos1 = gl_in[1].gl_Position;
    vec4 pos2 = gl_in[2].gl_Position;
    vec4 pos3 = gl_in[3].gl_Position;

    vec4 leftPos = pos0+v*(pos3-pos0);
    vec4 rightPos = pos1+v*(pos2-pos1);
    vec4 pos = leftPos+u*(rightPos-leftPos);
	
  gl_Position = cameras[0].proj * cameras[0].view * pos;
}