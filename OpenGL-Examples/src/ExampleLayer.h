#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

class ExampleLayer : public GLCore::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
	
private:

	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::OrthographicCameraController m_CameraController;

	int nCurvePoints = 4;
	std::vector<glm::vec2> curveControlPoints;
	std::vector<std::vector<glm::vec2>> surfaceControlPoints;
	
	GLuint m_QuadVA, m_QuadVB, m_QuadIB;

	int pointCount = 0;
	std::string buffer;

	float m_Step = 0.05f;
	int currentPoint = 0;
	int m_MaxDepth = 2;
	float m_Treshold = 0.1f;

private:
	glm::vec2 EvaluateCurve(float t, std::vector<glm::vec2>& points);
	glm::vec2 EvaluateSurface(float s, float t, std::vector<std::vector<glm::vec2>>& points);
	
	void DrawLine(glm::vec2& p1, glm::vec2& p2);
	void DrawPoint(glm::vec2 p, float size = 5);
	void DrawCurveControlLines();
	void DrawCurve(float step);
	void DrawSurface(float step);
	void TessellateCurve(float t0, float t1, glm::vec2& p0, glm::vec2& p1, int depth);
	void DrawTessellateCurve();
};