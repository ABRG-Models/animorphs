#version 410 core

// Output data
in vec2 UV;
out vec3 color;
in vec3 fragmentColor;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

uniform int type;

void main()
{

    if( type == 0 ) {
	    color = texture( myTextureSampler, UV ).rgb;
    } else {
        color = fragmentColor;
    }

}
