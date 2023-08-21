#version 330 core
out vec4 FragColor;

struct PointLight{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float c; //const
    float l; //linear
    float q; //quadratic
};


in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform PointLight pointLight;
uniform vec3 viewPosition;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = CalcPointLight(pointLight, normal, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

    //diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * light.diffuse * vec3(texture(texture_diffuse1, TexCoords));

    //specular Blinn-Phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0f);
    vec3 specular = spec * light.specular * vec3(texture(texture_specular1, TexCoords).xxx);

    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 /(light.c + light.l * distance + light.q * distance * distance);

    //ambient
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));

    return ((ambient + diffuse + specular) * attenuation);
}
