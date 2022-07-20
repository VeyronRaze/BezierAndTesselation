#version 450 core

layout (location = 0) out vec4 o_Color;

uniform vec4 u_Color;
uniform vec2 uPoints[9];
uniform vec2 testPoint;
uniform float uMarkerSize;

void main()
{
	o_Color = vec4(vec3(0.3f), 1.0f);

	for(int i = 0; i < uPoints.length(); i++)
		if(distance(gl_FragCoord.xy, uPoints[i]) < uMarkerSize)
			o_Color = vec4(0.7f, 0.0f, 0.0f, 1.0f);

	vec2 p0 = gl_FragCoord.xy;
	vec2 p1 = uPoints[0];
	vec2 p2 = uPoints[1];
	vec2 p3 = uPoints[2];

	float a = p3.x - 2 * p2.x + p1.x;
	float b = 2 * (p2.x - p1.x);
	float c = p1.x - p0.x;
	
	float dx = b * b - 4.0f * a * c;
	float u = -1.0f;
	
	if(a == 0.0f)
		u = -c / b;
	else if(dx >= 0.0f){
		float u1 = (-b + sqrt(dx)) / (2.0f * a);
		float u2 = (-b - sqrt(dx)) / (2.0f * a);
	
		if(u1 < 0.0f || u1 > 1.0f)
			u = u2;
		else if(u2 < 0.0f || u2 > 1.0f)
			u = u1;
		else
			u = min(u1, u2);
	}
	
	float y = -1.0f;
	if(u >= 0.0f && u <= 1.0f)
		y = u * u * (p3.y - 2 * p2.y + p1.y) + 2* u * (p2.y - p1.y) + p1.y;

	if(y == 0 || (distance(y, p0.y) < 1.0f))
		o_Color = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	if(distance(gl_FragCoord.xy, testPoint) < uMarkerSize)
			o_Color = vec4(0.0f, 0.0f, 0.7f, 1.0f);
}