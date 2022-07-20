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
	
	glm::vec4 getPointAndWeight(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	float solveQuadratic(float a, float b, float c);
	void getTextureCoord();

	float markerSize = 5.0f;
	int currPoint = 0;

	glm::vec2 testPoint = {0.0f, 0.0f};

	GLuint m_QuadVA, m_QuadVB, m_QuadIB;
	std::vector<glm::vec2> controlPoints = {
		{500.0f, 200.0f}, {700.0f, 200.0f}, {900.0f, 200.0f},
		{500.0f, 400.0f}, {700.0f, 400.0f}, {900.0f, 400.0f},
		{500.0f, 600.0f}, {700.0f, 600.0f}, {900.0f, 600.0f}
	};
};