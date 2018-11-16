#version 330 core
out vec4 fragColor;
uniform vec3 rgb;

void main() {
	fragColor = vec4(rgb, 1.0);
}
