#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <vector>
#include <UtH/UtHEngine.hpp>
#include <CavemanGame.h>
#include <map>
#include <list>
#include <fstream>
#include <TrailComponent.h>

using namespace uth;
using namespace std;

namespace uth
{
	class GameScene : public Scene
	{
	public:
		GameScene();
		~GameScene() override;

		bool Init() override;
		bool DeInit() override;

		void Update(float dt) override;
		void Draw(RenderTarget& target, RenderAttributes attributes = RenderAttributes()) override;

	private:
		// TODO:
		bool DeleteObjects(GameObject* p_object);
		Vec2 GetRandomSpawnPosition();
		// Logic for gameobject that are hit by caveman.
		void ReactToHit(GameObject* a);
		void MaintainObjectList(float dt);
		void AddObjects();
		void DeleteTrail(GameObject* p_trail);
		void UpdateBackground();
		void UpdateCameraMovement(float dt);
		void Input();
		void UpdateButtonPositions();
		void GameOverLogic();
		// Controls the amount of gameObjects in the scene.
		void GameState();

		void BackgroundInit();
		void PauseInit();
		void ButtonsInit();
		void ContactLogicInit();
		void ParticleInit();
		void VariableInit();
		void SoundInit();

		bool paused;

		// Physics and particles
		PhysicsContactListener contactListener;
		PhysicsWorld* p_world;
		ParticleSystem* p_partsys;
		float particleTimer;

		// Background
		GameObject* p_background[4];
		GameObject* overlay;
		float screenDiameter;

		// Game objects
		Caveman* p_caveman;
		Club* p_club;
		Arrow* p_arrow;
		multimap<string, GameObject*> objectList;
		multimap<string, GameObject*>::reverse_iterator i_ObjectList;
		list<GameObject*> trailList;
		list<GameObject*>::iterator i_trailList;

		PrefabObject prefabObject;
		short asteroidAmount;
		short astronautAmount;
		short cosmonautAmount;

		// Buttons
		Button* p_pauseButton;
		Button* p_playButton;

		// Sounds
		Sound* p_astroDie;
		Sound* p_astroHit;
		Sound* p_panic1;
		Sound* p_panic2;
		Sound* p_clubAttack;
		Sound* p_cavemanMove;
		Sound* p_hitMetal;
		Sound* p_hitRock;
		// TODO: add music!

		// Scorekeeping
		Statistics stats;
		std::fstream scorefile;

		int nauts;
	};
}

#endif