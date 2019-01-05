#version 330 core
out vec4 fragColor;
uniform vec3 rgb;
uniform vec3 cameraLightPos;

in vec3 FragPos;
in vec3 Normal;

void main() {
	const float gamma = 2.2;

	vec3 lightColor = vec3(1, 1, 1);

	// ambient
	vec3 ambient = 0.1 * lightColor;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(cameraLightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = 3.0 * diff * lightColor;

	// specular
	vec3 viewPos = cameraLightPos;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 256);
	vec3 specular = 0.5 * spec * lightColor;

	// attenuation
	float distance = max(length(cameraLightPos - FragPos) - 1, 1.0);
	float attenuation = 1.0 / (distance * distance);
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = ((ambient + diffuse) * max(rgb, 0)) + (specular * lightColor);

	// exposure tone mapping
	result = vec3(1.0) - exp(-result * 1.0);

	// gamma correction
	result = pow(result, vec3(1.0 / gamma));

	fragColor = vec4(result, 1.0);
}
