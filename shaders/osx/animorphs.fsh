#version 410 core

// Output data
in vec2 UV;
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main()
{

	color = texture( myTextureSampler, UV ).rgb;

}
