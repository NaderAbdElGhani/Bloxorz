
#version 330 core

in vec2 vUV;

uniform sampler2D tex;
uniform mat4 colortransform;
out vec4 color;

void main()
{
	    
	//function texture take a "sampler" and "uv" then return a vec4 containing the sampled color
        vec4 frag_color=texture(tex,vUV);
	vec4 clr={0.0f,0.0f,0.0f,1.0f};
	float elipse=(pow(gl_FragCoord.x-700/2,2)/(1.6*700*700/4));
	elipse+=(pow(gl_FragCoord.y-500/2,2)/(1.6*500*500/4));
	color=mix(frag_color,clr,abs(elipse));
        color=colortransform*color;
}

