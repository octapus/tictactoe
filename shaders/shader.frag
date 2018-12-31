#version 330 core
out vec4 fragColor;
uniform vec3 rgb;
uniform vec3 cameraLightPos;

in vec3 FragPos;
in vec3 Normal;

void main() {
	vec3 lightColor = vec3(1, 1, 1);

	vec3 ambient = 0.1 * lightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(cameraLightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 viewPos = cameraLightPos;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = 0.5 * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * rgb;
	fragColor = vec4(result, 1.0);
}
