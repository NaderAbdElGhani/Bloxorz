#version 330 core

in vec2 vUV;


in Interpolators {
    vec3 world;
    vec3 view;
    vec3 normal;
   
   
} fs_in;

struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};
uniform Material material;

struct DirectionalLight {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 position;
	vec3 direction;
 
};
uniform DirectionalLight light;
uniform sampler2D tex;
uniform mat4 colortransform;

out vec4 color;
float diffuse(vec3 n, vec3 l){
    //Diffuse (Lambert) term computation: reflected light = cosine the light incidence angle on the surface
    //max(0, ..) is used since light shouldn't be negative
    return max(0, dot(n,l));
}

float specular(vec3 n, vec3 l, vec3 v, float shininess){
    //Phong Specular term computation
    return pow(max(0, dot(v,reflect(-l, n))), shininess);
}
void main()
{ vec4 color1,color2 ;
    //function texture take a "sampler" and "uv" then return a vec4 containing the sampled color
    color1 = texture(tex, vUV);

	vec3 n = normalize(fs_in.normal);
    vec3 v = normalize(fs_in.view);
  color2 = vec4(
        material.ambient*light.ambient + 
        material.diffuse*light.diffuse*diffuse(n,- light.direction) + 
        material.specular*light.specular*specular(n, -light.direction, v, material.shininess),
        1.0f
    );
color=color2*color1*colortransform;
}