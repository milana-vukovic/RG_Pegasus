#version 330 core
out vec4 FragColor;



uniform float p;
uniform vec3 lightColor;

void main(){
    //Procenat druge pisemo
    //Relektujemo u odnosu n y osu
    //FragColor = mix(texture(t0, outTexCords), texture(t1, vec2(1.0 - outTexCords.x, outTexCords.y)), p) * vec4(outCol,1.0);
    //FragColor = mix(texture(t0, vec2(outTexCords.x/2.0, outTexCords.y/2.0)), texture(t1, outTexCords), p) * vec4(outCol,1.0);
    //FragColor = mix(texture(t0, outTexCords), texture(t1, outTexCords), p);
    FragColor = vec4(lightColor, 1.0);
}