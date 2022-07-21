#include "ExampleLayer.h"
#include <cmath>

using namespace GLCore;
using namespace GLCore::Utils;

ExampleLayer::ExampleLayer()
	: m_CameraController(16.0f / 9.0f)
{
	surfaceControlPoints = {
		{{-1.0f, -1.0f}, {0.0f, -1.0f}, {1.0f, -1.0f}},
		{{-1.0f,  0.0f}, {0.0f,  0.0f}, {1.0f,  0.0f}},
		{{-1.0f,  1.0f}, {0.0f,  1.0f}, {1.0f,  1.0f}}
	};

	curveControlPoints = {{-1.5f, 0.5f}, {-0.5f, 1.0f}, {0.5f, -1.0f}, {1.0f, 0.0f}};
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

	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

	uint32_t indices[] = { 0, 1};
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

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());

	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	glBindVertexArray(m_QuadVA);

	//DrawTessellateCurve();
	//DrawCurve(m_Step);
	//DrawCurveControlLines();
	
	DrawSurface(m_Step);
	
	//glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, nullptr);
}

void ExampleLayer::OnImGuiRender()
{
	ImGui::Begin("Controls");
	ImGui::DragInt("Max Tessellation Depth", &m_MaxDepth, 0.01f, 1, 10);
	ImGui::DragFloat("Curve Resolution", &m_Step, 0.001f, 0.0001f, 1.0f, "%.4f");
	ImGui::DragInt("Current Point", &currentPoint, 0.05f, 0, 3);
	ImGui::DragFloat2("Position", glm::value_ptr(curveControlPoints[currentPoint]), 0.01f);
	ImGui::Text(("Point count: " + std::to_string(pointCount)).c_str());
	ImGui::End();
}

void ExampleLayer::DrawLine(glm::vec2& p1, glm::vec2& p2){
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2), glm::value_ptr(p1));
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec2), sizeof(glm::vec2), glm::value_ptr(p2));

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
}

void ExampleLayer::DrawPoint(glm::vec2 p, float size){
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2), glm::value_ptr(p));
	glPointSize(size / m_CameraController.GetZoomLevel());
	glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, nullptr);
}

float lerp(float x, float y, float t){
	return x + (y - x) * t;
}

glm::vec2 lerp(glm::vec2 p1, glm::vec2 p2, float t){
	return {lerp(p1.x, p2.x, t), lerp(p1.y, p2.y, t)};
};

glm::vec2 ExampleLayer::EvaluateCurve(float t, std::vector<glm::vec2>& points){
	size_t degree = points.size();
	std::vector<glm::vec2> localPoints = points;
	while(degree > 1){
		for(size_t i = 0; i < degree - 1; i++)
			localPoints[i] = lerp(localPoints[i], localPoints[i + 1], t);
		degree--;
	}

	return localPoints[0];
}

glm::vec2 ExampleLayer::EvaluateSurface(float s, float t, std::vector<std::vector<glm::vec2>>& points){
	std::vector<glm::vec2> localPoints;
	for(std::vector<glm::vec2> curve : points)
		localPoints.push_back(EvaluateCurve(t, curve));

	return EvaluateCurve(s, localPoints);
}

void ExampleLayer::DrawCurveControlLines(){
	uint32_t location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	glUniform4fv(location, 1, glm::value_ptr(color));

	for(size_t i = 1; i < curveControlPoints.size(); i++)
		DrawLine(curveControlPoints[i - 1], curveControlPoints[i]);

	color = {1.0f, 0.0f, 0.0f, 1.0f};
	glUniform4fv(location, 1, glm::value_ptr(color));

	for(auto& controlPoint : curveControlPoints)
		DrawPoint(controlPoint, 10);
}

void ExampleLayer::DrawCurve(float step){
	uint32_t location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glm::vec4 color = {0.0f, 0.0f, 1.0f, 1.0f};
	glUniform4fv(location, 1, glm::value_ptr(color));

	if(step <= 0.0f)
		return;

	pointCount = 0;

	for(float t = 0.0f; t <= 1.0f; t += step){
		DrawPoint(EvaluateCurve(t, curveControlPoints));
		pointCount++;
	}
}

void ExampleLayer::DrawSurface(float step){
	uint32_t location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	glUniform4fv(location, 1, glm::value_ptr(color));

	if(step <= 0.0f)
		return;

	pointCount = 0;

	for(float s = 0.0f; s <= 1.0f; s += step)
		for(float t = 0.0f; t <= 1.0f; t += step){
			DrawPoint(EvaluateSurface(s, t, surfaceControlPoints));
			pointCount++;
		}
}

void ExampleLayer::TessellateCurve(float t0, float t1, glm::vec2& p0, glm::vec2& p1, int depth){
	if(depth >= m_MaxDepth){
		DrawLine(p0, p1);
		return;
	}
	
	float tOneQuarter = (t1 + t0) * 0.25f;
	float tMidpoint = (t1 + t0) * 0.5f;
	float tThreeQuarter = (t1 + t0) * 0.75f;

	glm::vec2 cOneQuarter = EvaluateCurve(tOneQuarter, curveControlPoints);
	glm::vec2 cMidpoint = EvaluateCurve(tMidpoint, curveControlPoints);
	glm::vec2 cThreeQuarter = EvaluateCurve(tThreeQuarter, curveControlPoints);

	glm::vec2 pOneQuarter = (p0 + p1) * 0.25f;
	glm::vec2 pMidpoint = (p0 + p1) * 0.5f;
	glm::vec2 pThreeQuarter = (p0 + p1) * 0.75f;

	if(glm::distance(cOneQuarter, pOneQuarter) > m_Treshold ||
	   glm::distance(cMidpoint, pMidpoint) > m_Treshold ||
	   glm::distance(cThreeQuarter, pThreeQuarter) > m_Treshold){
		TessellateCurve(t0, tMidpoint, p0, cMidpoint, depth + 1);
		TessellateCurve(tMidpoint, t1, cMidpoint, p1, depth + 1);
	}
	else
		DrawLine(p0, p1);
}

void ExampleLayer::DrawTessellateCurve(){
	TessellateCurve(0.0f, 1.0f, curveControlPoints.front(), curveControlPoints.back(), 0);
}
