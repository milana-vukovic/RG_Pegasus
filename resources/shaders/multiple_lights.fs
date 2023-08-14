// 1 dir
// 4 point
// 1 spotlight
#version 330 core
struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float c; //const
    float l; //linear
    float q; //quadratic
};

struct SpotLight{
    vec3 position;
    vec3 direction;

    float cutOff; //cos
    float outerCutOff; //cos

    float c;
    float l;
    float q;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

};

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;

};
#define NR_POINT_LIGHT 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHT];
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform Material material;


out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


void main(){

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    for(int i = 0; i < NR_POINT_LIGHT; i++){
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    //result += vec3(texture(material.emission, TexCoords));

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){

    //diffuse
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoords));

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoords));

    //ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    return (ambient + diffuse + specular);

}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

    //diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoords));

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoords));

    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 /(light.c + light.l * distance + light.q * distance * distance);

    //ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    return ((ambient + diffuse + specular) * attenuation);
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

    //diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoords));

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoords));

    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 /(light.c + light.l * distance + light.q * distance * distance);

    float cosTheta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((cosTheta - light.outerCutOff) / epsilon, 0.0, 1.0);

    //ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    return ((ambient + diffuse + specular) * attenuation * intensity);
}