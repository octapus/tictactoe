#version 330 core
out vec4 fragColor;
uniform vec3 rgb;
uniform vec3 cameraLightPos;

in vec3 FragPos;
in vec3 Normal;

void main() {
	vec3 ambient = 0.1 * vec3(1, 1, 1);

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(cameraLightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1, 1, 1);

	vec3 result = (ambient + diffuse) * rgb;
	fragColor = vec4(result, 1.0);
}
