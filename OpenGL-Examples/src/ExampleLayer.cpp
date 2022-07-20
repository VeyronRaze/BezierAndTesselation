#include "ExampleLayer.h"

using namespace GLCore;
using namespace GLCore::Utils;

ExampleLayer::ExampleLayer()
	: m_CameraController(16.0f / 9.0f)
{

}

ExampleLayer::~ExampleLayer()
{

}

void ExampleLayer::OnAttach()
{
	EnableGLDebugging();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);

	glCreateVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};

	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	uint32_t indices[] = {0, 1, 2, 2, 3, 0};
	glCreateBuffers(1, &m_QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void ExampleLayer::OnDetach()
{
	glDeleteVertexArrays(1, &m_QuadVA);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);
}

void ExampleLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);
}

void ExampleLayer::OnUpdate(Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	getTextureCoord();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());

	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	GLfloat color[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glUniform4fv(location, 1, color);

	location = glGetUniformLocation(m_Shader->GetRendererID(), "uPoints");
	glUniform2fv(location, 9,  glm::value_ptr(*controlPoints.data()));

	location = glGetUniformLocation(m_Shader->GetRendererID(), "testPoint");
	glUniform2fv(location, 1, glm::value_ptr(testPoint));

	location = glGetUniformLocation(m_Shader->GetRendererID(), "uMarkerSize");
	glUniform1f(location, markerSize);

	glBindVertexArray(m_QuadVA);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void ExampleLayer::OnImGuiRender()
{
	ImGui::Begin("Controls");
	ImGui::DragInt("Current point", &currPoint, 0.01f, 0, 8);
	ImGui::DragFloat2("Coords", &controlPoints[currPoint][0], 0.1f);
	ImGui::DragFloat("Marker size", &markerSize, 0.01f, 0.0f);
	ImGui::DragFloat2("Test Point", glm::value_ptr(testPoint), 0.5f);
	ImGui::End();
}

glm::vec4 ExampleLayer::getPointAndWeight(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3){
	float ax = p3.x - 2 * p2.x + p1.x;
	float bx = 2 * (p2.x - p1.x);
	float cx = p1.x - p0.x;

	float ay = p3.y - 2 * p2.y + p1.y;
	float by = 2 * (p2.y - p1.y);
	float cy = p1.y - p0.y;

	glm::vec2 a = p3 - 2.0f * p2 + p1;
	glm::vec2 b = 2.0f * (p2 - p1);
	glm::vec2 c = p1 - p0;
	glm::vec2 d = b * b - 4.0f * a * c;

	glm::vec2 u;
	glm::vec2 u1;
	glm::vec2 u2;

	if(a == glm::vec2(0.0f))
		u = -c / b;
	else{
		u1 = (-b + sqrt(d)) / (2.0f * a);
		u2 = (-b - sqrt(d)) / (2.0f * a);
	}

	glm::vec2 pp1 = u1 * u1 * a + 2.0f * u1 * b + p1;
	glm::vec2 pp2 = u2 * u2 * a + 2.0f * u2 * b + p1;

	float ux = solveQuadratic(ax, bx, cx);
	float uy = solveQuadratic(ay, by, cy);
	
	float y = -1.0f;
	float x = -1.0f;
	if(ux >= 0.0f && ux <= 1.0f)
		y = ux * ux * ay + 2 * ux * by + p1.y;

	if(uy >= 0.0f && uy <= 1.0f)
		x = uy * uy * ax + 2* uy * bx + p1.x;

	return {x, y, ux, uy};
}

float ExampleLayer::solveQuadratic(float a, float b, float c){
	float d = b * b - 4.0f * a * c;
	float u = -1.0f;

	//if(a == 0.0f && b != 0.0f)
	//	u = -c / b;
	//else if(d >= 0.0f){
	//	float u1 = (-b + sqrt(d)) / (2.0f * a);
	//	float u2 = (-b - sqrt(d)) / (2.0f * a);
	//
	//	if(u1 < 0.0f || u1 > 1.0f)
	//		u = u2;
	//	else if(u2 < 0.0f || u2 > 1.0f)
	//		u = u1;
	//	else
	//		u = std::min(u1, u2);
	//}

	if(a == 0.0f)
		u = -c / b;
	else if(d >= 0.0f){
		float u1 = (-b + sqrt(d)) / (2.0f * a);
		float u2 = (-b - sqrt(d)) / (2.0f * a);

		if(u1 < 0.0f || u1 > 1.0f)
			u = u2;
		else if(u2 < 0.0f || u2 > 1.0f)
			u = u1;
		else
			u = std::min(u1, u2);
	}

	return u;
}

void ExampleLayer::getTextureCoord(){
	glm::vec4 p1 = getPointAndWeight(testPoint, controlPoints[0], controlPoints[1], controlPoints[2]);
	std::cout << "p1[x: " << p1.x << ", y: " << p1.y << ", ux: " << p1.z << ", uy: " << p1.w << "]\n";
	//glm::vec4 p2 = getPointAndWeight(testPoint, controlPoints[3], controlPoints[4], controlPoints[5]);
	//std::cout << "p2[x: " << p2.x << ", y: " << p2.y << ", ux: " << p2.z << ", uy: " << p2.w << "], ";
	//glm::vec4 p3 = getPointAndWeight(testPoint, controlPoints[6], controlPoints[7], controlPoints[8]);
	//std::cout << "p3[x: " << p3.x << ", y: " << p3.y << ", ux: " << p3.z << ", uy: " << p3.w << "]\n";
}
