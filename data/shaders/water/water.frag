#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) flat in uint idx;
layout(location = 1) in vec3 interpolatedNormal;
layout(location = 2) in vec2 interpolatedTexCoord;
layout(location = 3) in vec3 viewDir;
layout(location = 4) in vec3 lightDir;
layout(location = 5) in float lightDistance2;
layout(location = 6) in vec3 fragPos;

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
    Light light = lights[0];
    Material material = materials[0];
    Camera camera = cameras[0];

    vec3 norm = normalize(interpolatedNormal);
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    vec3 viewDir = normalize(camera.position.xyz - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular.w * spec * light.diffuse.xyz; 
    
    float diff = max(dot(norm, light.direction.xyz), 0.0);
    vec3 diffuse = diff * light.diffuse.xyz;

    vec3 result = (((light.ambient.xyz*material.ambient.xyz) + diffuse + specular) * material.diffuse.xyz);
    outColor = vec4(0.0,1.0,1.0, 0.5);


    //outColor = material.diffuse;

}


