#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "imfilebrowser.h"
#include <gl2d/gl2d.h>
#include <platformTools.h>

class Boid
{
public:
	glm::vec2 position;
	glm::vec2 velocity;

	void update(float deltaTime);
};

void Boid::update(float deltaTime)
{
	this->position += velocity * deltaTime;
}

const int MAX_BOIDS = 10000;
struct GameData
{
	int spawnCount = 100;
	float velocityMin;
	float velocityMax;

	float turnFactor = 1;

	Boid boids[MAX_BOIDS];
	int boidsCount;

} gameData;

gl2d::Renderer2D renderer;

glm::vec2 randomVec2(int w, int h)
{
	return glm::vec2(rand() % w, rand() % h);
}

glm::vec2 randomVec2(float low, float hi)
{
	glm::vec2 v = glm::vec2(float(rand()) / RAND_MAX, float(rand()) / RAND_MAX);
	return v * glm::vec2(hi + 1 - low, hi + 1 - low) + glm::vec2(low, low);
}

bool initGame()
{
	// initializing stuff for the renderer
	gl2d::init();
	renderer.create();

	// loading the saved data. Loading an entire structure like this makes savind game data very easy.
	platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

	std::srand(std::time(0));

	gameData.boidsCount = 0;

	return true;
}

void spawnBoids(int count)
{
	if (count + gameData.boidsCount <= MAX_BOIDS)
		for (int i = 0; i < count; i++)
		{
			glm::vec2 v = randomVec2(gameData.velocityMin, gameData.velocityMax);
			gameData.boids[gameData.boidsCount++] = {randomVec2(renderer.windowW, renderer.windowH), v};
		}
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0;
	int h = 0;
	w = platform::getFrameBufferSizeX(); // window w
	h = platform::getFrameBufferSizeY(); // window h

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT); // clear screen

	renderer.updateWindowMetrics(w, h);
#pragma endregion

	if (platform::isButtonPressedOn(platform::Button::Escape))
	{
		return false;
	}

	if (platform::isButtonPressedOn(platform::Button::F))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}

	// update and render wooo
	for (int i = 0; i < gameData.boidsCount; i++)
	{
		if (gameData.boids[i].position.x <= 0) {
			gameData.boids[i].velocity.x += gameData.turnFactor;
		} else if (gameData.boids[i].position.x >= renderer.windowW) {
			gameData.boids[i].velocity.x += -gameData.turnFactor;
		} else if (gameData.boids[i].position.y <= 0) {
			gameData.boids[i].velocity.y += gameData.turnFactor;
		} else if (gameData.boids[i].position.y >= renderer.windowH) {
			gameData.boids[i].velocity.y += -gameData.turnFactor;
		}


		gameData.boids[i].update(deltaTime);

		renderer.renderCircleOutline(gameData.boids[i].position, Colors_White, 1, 1);
	}

	if (gameData.spawnCount > MAX_BOIDS - gameData.boidsCount)
	{
		gameData.spawnCount = MAX_BOIDS - gameData.boidsCount;
	}

	ImGui::Begin("Data", false, ImGuiWindowFlags_MenuBar);
	ImGui::LabelText("deltaTime", "%0.4fms", deltaTime * 1000);
	ImGui::LabelText("fps", "%0.0f", 1 / deltaTime);
	ImGui::LabelText("Amount Of Boids", "%d", gameData.boidsCount);
	ImGui::DragInt("spawn count", &gameData.spawnCount, 1, 1, MAX_BOIDS - gameData.boidsCount);
	ImGui::DragFloatRange2("velocity", &gameData.velocityMin, &gameData.velocityMax, 10, -100, 100);
	ImGui::DragFloat("turn factor", &gameData.turnFactor, 1, 1, 10);

	if (ImGui::Button("spawn boids", ImVec2{100, 50}) || platform::isButtonPressedOn(platform::Button::Space))
	{
		spawnBoids(gameData.spawnCount);
	};
	if (ImGui::Button("clear boids", ImVec2{100, 50}))
	{
		gameData.boidsCount = 0;
	};
	ImGui::End();

	// ImGui::ShowDemoWindow();

	renderer.flush();

	return true;
#pragma endregion
}

// This function might not be be called if the program is forced closed
void closeGame()
{

	// saved the data.
	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));
}
