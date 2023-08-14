#version 330 core
out vec4 FragColor;

//TODO: Material
struct Material{
    vec3 ambient;
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 direction;
    vec3 lightPos;

    float c; //constant
    float l; //linear
    float q; //quadratic

    float cutOff; //cos (phi)
    float outerCutOff; //cos (gama)
};

in vec2 outTexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos; //pozicija kamere

uniform Material material;
uniform Light light;
//uniform sampler2D t0;
//uniform sampler2D t1;


void main(){

    //FragColor = mix(texture(t0, outTexCords), texture(t1, vec2(1.0 - outTexCords.x, outTexCords.y)), p) * vec4(outCol,1.0);
    //FragColor = mix(texture(t0, vec2(outTexCords.x/2.0, outTexCords.y/2.0)), texture(t1, outTexCords), p) * vec4(outCol,1.0);
    //FragColor = mix(texture(t0, outTexCords), texture(t1, outTexCords), p);

    //TODO: spotlight
    vec3 lightDir =  normalize(light.lightPos - FragPos);
    float cosTheta = dot(normalize(light.direction), -lightDir);
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((cosTheta - light.outerCutOff) / epsilon, 0.0, 1.0);


    //TODO: ambient ako nemamo amijentalnu mapu
    vec3 ambient = texture(material.diffuse, outTexCoords).rgb * light.ambient;

    //TODO: diffuse
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(light.lightPos - FragPos);
    //vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = texture(material.diffuse, outTexCoords).rgb * diff * light.diffuse;

    //TODO: specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    // podizemo na stepen i tako uticemo na shininess
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
    vec3 specular = (texture(material.specular, outTexCoords).rgb * spec) * light.specular;

    //TODO: attenuation
    float distance = length(light.lightPos - FragPos);
    float attenuation = 1.0 / (light.c + light.l * distance + light.q * distance * distance );

    vec3 result = (ambient + (diffuse + specular) * intensity) * attenuation;
    FragColor = vec4(result, 1.0);

}