#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 ModelPos;
out vec3 Normal;
out mat4 TransformedModel;

out vec3 ViewPos;
out vec3 ViewNormal;

out mat3 TBN;
out mat3 ViewTBN;
out vec3 TangentViewerPos;
out vec3 TangentFragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 u_meshOffset;
uniform vec3 viewerPos;

void main()
{
    TexCoords = aTexCoords;
    TransformedModel = model * u_meshOffset;
    WorldPos = vec3(TransformedModel * vec4(aPos, 1.0));
    ModelPos = aPos;
    ViewPos = (view * vec4(WorldPos, 1.0)).xyz;

    gl_Position = projection * vec4(ViewPos, 1.0);

    // world space normals
    mat3 normalMatrix = transpose(inverse(mat3(TransformedModel)));
    Normal = normalize(normalMatrix * aNormal);

    vec3 T = normalize(normalMatrix * aTangent);
    T = normalize(T - dot(T, Normal) * Normal);
    vec3 B = cross(Normal, T);
    TBN = mat3(T, B, Normal);

    TangentViewerPos = TBN * viewerPos;
    TangentFragPos = TBN * WorldPos;

    // view space normals
    mat3 viewNormalMatrix = transpose(inverse(mat3(view * TransformedModel)));
    ViewNormal = normalize(viewNormalMatrix * aNormal);

    vec3 viewT = normalize(viewNormalMatrix * aTangent);
    viewT = normalize(viewT - dot(viewT, ViewNormal) * ViewNormal);
    vec3 viewB = cross(ViewNormal, viewT);
    ViewTBN = mat3(viewT, viewB, ViewNormal);
}