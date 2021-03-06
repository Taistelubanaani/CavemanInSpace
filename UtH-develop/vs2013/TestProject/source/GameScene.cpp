#include <GameScene.h>

GameScene::GameScene()
	: stats(),
	prefabObject()
{}
GameScene::~GameScene() 
{
}

bool GameScene::Init()
{	
	paused = true;
	VariableInit();
	ParticleInit();
	ContactLogicInit();
	ButtonsInit();
	PauseInit();
	BackgroundInit();
	SoundInit();
	paused = false;
	return true;
}
bool GameScene::DeInit()
{
	for (i_ObjectList = objectList.rbegin(); i_ObjectList != objectList.rend(); ++i_ObjectList)
	{
		delete(i_ObjectList->second);
	}

	delete p_world;

	for (int i = 0; i < 4; ++i)
	{
		delete p_background[i];
	}

	delete p_pauseButton;
	delete overlay;
	delete p_replayButton;
	delete p_playButton;
	delete p_partsys;
	delete p_rockpartsys;

	return true;
}
void GameScene::Update(float dt)
{
	if (!paused)
	{
		Scene::Update(dt);
		
		UpdateBackground();
		p_world->Update(dt);
		if (!isDying)
		{
			p_club->update(dt, p_caveman->transform.GetPosition());
		}
		stats.Update(dt);

		if (particleTimer > 0)
		{
			particleTimer--;
		}
		else
		{
			p_rockpartsys->SetEmitProperties(false);
			p_partsys->SetEmitProperties(false);
			particleTimer = 0;
		}
		p_partsys->RaiseUpdateFlag();
		p_partsys->Update(dt);
		p_rockpartsys->RaiseUpdateFlag();
		p_rockpartsys->Update(dt);

		MaintainObjectList(dt);

		if (deleteAsteroid != nullptr)
			deleteAsteroid->Update(dt);

		DeleteAsteroid(dt);
		UpdateCameraMovement(dt);
		p_pauseButton->Update(dt);
		Input();
		
	}

	UpdateButtonPositions();
	p_playButton->Update(dt);
	p_replayButton->Update(dt);
	if (stats.IsDead())
	{
		if (!isDying)
		{
			p_caveman->Die();
			isDying = true;
		}
		deathTimer -= dt;
		if (deathTimer <= 0)
		{
			GameOverLogic();
		}
	}
}
void GameScene::Draw(RenderTarget& target, RenderAttributes attributes)
{
	for (int i = 0; i < 4; ++i)
	{
		p_background[i]->Draw(target, attributes);
	}

	Scene::Draw(target, attributes);
	p_partsys->Draw(target, attributes);
	p_rockpartsys->Draw(target, attributes);

	if (deleteAsteroid != nullptr)
		deleteAsteroid->Draw(target, attributes);

	for (i_ObjectList = objectList.rbegin(); i_ObjectList != objectList.rend(); ++i_ObjectList)
	{
		i_ObjectList->second->Draw(target, attributes);
	}

	p_pauseButton->Draw(target, attributes);
	stats.Draw(target, attributes);
	overlay->Draw(target, attributes);
	p_playButton->Draw(target, attributes);
	p_replayButton->Draw(target, attributes);

}

// Private //

void GameScene::ReactToHit(GameObject* a)
{
	for (i_ObjectList = objectList.rbegin(); i_ObjectList != objectList.rend(); ++i_ObjectList)
	{
		if (i_ObjectList->second == a)
		{
			if (i_ObjectList->first == "Astronaut")
			{
				a->GetComponent<NautComponent>()->Hit(p_club->transform.GetPosition());

				//kill astronaut
				if (a->GetComponent<NautComponent>()->isDead() && a->GetComponent<NautComponent>()->oxygen > 0)
				{
					p_astroDie->PlayEffect();
					stats.addOxygen += a->GetComponent<NautComponent>()->oxygen;
					a->GetComponent<NautComponent>()->oxygen = 0;
					stats.addScore += 100;
					//particles!
					p_partsys->transform.SetPosition(a->GetComponent<Rigidbody>()->GetPosition());
					p_partsys->SetEmitProperties(true, 0, 0.2f, 1, 3);
					particleTimer = 50;
				}
			}
			else if (i_ObjectList->first == "Cosmonaut")
			{
				a->GetComponent<NautComponent>()->Hit(p_club->transform.GetPosition());

				if (a->GetComponent<NautComponent>()->isDead() && a->GetComponent<NautComponent>()->oxygen > 0)
				{
					p_astroDie->PlayEffect();
					stats.addOxygen += a->GetComponent<NautComponent>()->oxygen;
					a->GetComponent<NautComponent>()->oxygen = 0;
					stats.addScore += 200;
					//particles!
					p_partsys->transform.SetPosition(a->GetComponent<Rigidbody>()->GetPosition());
					p_partsys->SetEmitProperties(true, 0, 0.2f, 4, 6);
					particleTimer = 50;
				}
				else p_astroHit->PlayEffect();
			}
			else if (i_ObjectList->first == "Asteroid")
			{
				p_hitRock->PlayEffect();
				stats.addScore += 10;
				p_rockpartsys->transform.SetPosition(a->GetComponent<Rigidbody>()->GetPosition());
				p_rockpartsys->SetEmitProperties(true, 0, 0.7f, 1, 2);
				
				particleTimer = 50;
				deleteAsteroid = a;
				objectList.erase(--(i_ObjectList.base()));
			}

			break;
		}
	}
}
void GameScene::MaintainObjectList(float dt)
{
	// Updates, deletes and instantiates objects.

	GameState();
	AddObjects();
	//TraceNauts();

	if (objectList.size() > 0)
	{
		for (i_ObjectList = objectList.rbegin(); i_ObjectList != objectList.rend(); ++i_ObjectList)
		{
			i_ObjectList->second->Update(dt);

			if (DeleteObjects(i_ObjectList->second))
			{
				objectList.erase(--(i_ObjectList.base()));
				++nauts;
				std::cout << "Naut " << nauts << " deleted" << std::endl;
			}
		}
	}
}
void GameScene::AddObjects()
{

	// Maintains the objectlist so, that it always holds a certain amount of specified objects.
	
	while (objectList.count("Astronaut") < astronautAmount)
	{
		objectList.insert(make_pair("Astronaut", prefabObject.CreateGameObject(p_world, GetRandomSpawnPosition(), "AstronautAnim.png", "Astronaut")));
	}
	while (objectList.count("Cosmonaut") < cosmonautAmount)
	{
		objectList.insert(make_pair("Cosmonaut", prefabObject.CreateGameObject(p_world, GetRandomSpawnPosition(), "CosmonautAnim.png", "Cosmonaut")));
	}
	while (objectList.count("Asteroid") < asteroidAmount)
	{
		if (Randomizer::GetInt(0, 2) == 0)
			objectList.insert(make_pair("Asteroid", prefabObject.CreateGameObject(p_world, GetRandomSpawnPosition(), "Placeholders/Asteroid_3.png", "Asteroid")));
		else
			objectList.insert(make_pair("Asteroid", prefabObject.CreateGameObject(p_world, GetRandomSpawnPosition(), "Placeholders/Asteroid_4.png", "Asteroid")));
	}
}
bool GameScene::DeleteObjects(GameObject* p_object)
{
	// Deletes objects that are too far away from the player. (Return's true if the object was deleted.)
	if (p_object->HasTag("Naut"))
	{
		if ((p_object->GetComponent<NautComponent>()->isDead() && p_object->GetComponent<Sprite>()->GetColor().a <= 0))
		{
			delete(p_object);
			return true;
		}
	}
	
	if (Vec2::distance(p_object->transform.GetPosition(), p_caveman->transform.GetPosition()) >= screenDiameter)
	{
		delete(p_object);
		return true;
	}
	return false;
}
void GameScene::DeleteAsteroid(float dt)
{
	if (deleteAsteroid != nullptr)
	{
		deleteAsteroid->GetComponent<Rigidbody>("Rigidbody")->SetKinematic(true);

		if (deleteAsteroid->GetComponent<Sprite>()->GetColor().a > 0)
		{
			deleteAsteroid->GetComponent<Sprite>()->SetColor(1, 1, 1, (deleteAsteroid->GetComponent<Sprite>()->GetColor().a) - 2 / (1 / dt));
			deleteAsteroid->GetComponent<Rigidbody>()->SetKinematic(true);
		}
		else
		{
			delete(deleteAsteroid);
			deleteAsteroid = nullptr;
		}
	}
}
Vec2 GameScene::GetRandomSpawnPosition()
{
	// Returns a random position outside the field of view, but not too far away.

	short randomNumber = Randomizer::GetInt(0, 4);
	Vec2 temp;

	do {
		switch (randomNumber)
		{
		case 0: temp = Vec2(Randomizer::GetFloat(0, 1), Randomizer::GetFloat(0, 1)); break;
		case 1: temp = Vec2(Randomizer::GetFloat(0, 1), Randomizer::GetFloat(-1, 0)); break;
		case 2: temp = Vec2(Randomizer::GetFloat(-1, 0), Randomizer::GetFloat(-1, 0)); break;
		case 3: temp = Vec2(Randomizer::GetFloat(-1, 0), Randomizer::GetFloat(0, 1)); break;
		default: temp = Vec2(Randomizer::GetFloat(0, 1), Randomizer::GetFloat(0, 1)); break;
		}
	} while (temp.length() == 0);

	temp.normalize();
	temp *= screenDiameter * 0.66f;
	return temp + p_caveman->transform.GetPosition();
}
void GameScene::UpdateBackground()
{
	// Moves the background objects according to the player's movement to create an illusion of an endless background.

	for (int i = 0; i < 4; ++i)
	{
		if (p_background[i]->transform.GetPosition().x - uthEngine.GetWindow().GetCamera().GetPosition().x < p_background[1]->GetComponent<Sprite>()->GetSize().x * -1)
			p_background[i]->transform.SetPosition(p_background[i]->transform.GetPosition().x + p_background[1]->GetComponent<Sprite>()->GetSize().x * 2, p_background[i]->transform.GetPosition().y);
		else if (p_background[i]->transform.GetPosition().x - uthEngine.GetWindow().GetCamera().GetPosition().x > p_background[1]->GetComponent<Sprite>()->GetSize().x)
			p_background[i]->transform.SetPosition(p_background[i]->transform.GetPosition().x - p_background[1]->GetComponent<Sprite>()->GetSize().x * 2, p_background[i]->transform.GetPosition().y);

		if (p_background[i]->transform.GetPosition().y - uthEngine.GetWindow().GetCamera().GetPosition().y < p_background[1]->GetComponent<Sprite>()->GetSize().y * -1)
			p_background[i]->transform.SetPosition(p_background[i]->transform.GetPosition().x, p_background[i]->transform.GetPosition().y + p_background[1]->GetComponent<Sprite>()->GetSize().y * 2);
		else if (p_background[i]->transform.GetPosition().y - uthEngine.GetWindow().GetCamera().GetPosition().y > p_background[1]->GetComponent<Sprite>()->GetSize().y)
			p_background[i]->transform.SetPosition(p_background[i]->transform.GetPosition().x, p_background[i]->transform.GetPosition().y - p_background[1]->GetComponent<Sprite>()->GetSize().y * 2);
	}
}
void GameScene::UpdateCameraMovement(float dt)
{
	// Makes the camera smoothly follow the player.
	
	Vec2 movement = p_caveman->transform.GetPosition() - uthEngine.GetWindow().GetCamera().GetPosition();

	if (movement.length() != 0)
		movement.normalize();

	if (Vec2::distance(p_caveman->transform.GetPosition(), uthEngine.GetWindow().GetCamera().GetPosition()) >= 0.5f)
		uthEngine.GetWindow().GetCamera().Scroll(movement * Vec2::distance(p_caveman->transform.GetPosition(), uthEngine.GetWindow().GetCamera().GetPosition()) / 50.f);
	else
		uthEngine.GetWindow().GetCamera().SetPosition(p_caveman->transform.GetPosition());

	uthEngine.GetWindow().GetCamera().Update(dt);
}
void GameScene::Input()
{
	// TODO: check which input version should be used?

	// Handles touch input.
	if (!isDying)
	{
		if (uthInput.Touch[0].Motion() == TouchMotion::RELEASE &&
			p_arrow->IsActive() &&
			Vec2::distance(uthInput.Touch[0].GetStartPosition(), uthInput.Touch[0].GetEndPosition()) > 30)
		{
			bool bigpull = p_arrow->IsStrong();
			p_caveman->ChangeDirectionMouse(p_arrow->GetNormDirection(), bigpull);
			if (bigpull)
			{
				stats.addOxygen -= 0.1f;
				p_cavemanMove->SetVolume(100); // OR SetPitch?
			}
			else
			{
				stats.addOxygen -= 0.05f;
				p_cavemanMove->SetVolume(70);
			}
			p_arrow->DisableArrow();
			//p_caveman->ChangeDirectionTouch(uthInput.Touch[0].GetStartPosition(), uthInput.Touch[0].GetEndPosition());
		}
		else if (uthInput.Touch[0].Motion() == TouchMotion::DRAG && Vec2::distance(uthInput.Touch[0].GetStartPosition(), uthInput.Common.Position()) > 30)
		{
			if (p_arrow->IsActive() == false)
				p_arrow->DrawArrow(uthInput.Common.Position());
			else p_arrow->update(p_caveman->transform.GetPosition());
		}
		else if (uthInput.Touch[0].Motion() == TouchMotion::TAP && p_club->isHitting() == false)
		{
			p_clubAttack->PlayEffect();
			Vec2 hitPoint = uthEngine.GetWindow().PixelToCoords(uthInput.Common.Position());
			p_club->Hit(p_caveman->transform.GetPosition(), hitPoint);
			p_caveman->Hit(hitPoint, isDying);
		}
	}
	
	// Mouse input for testing.
	//if (uthInput.Mouse.IsButtonDown(Mouse::MButton::LEFT) == true && p_arrow->IsActive() == false)
	//	p_arrow->DrawArrow(uthInput.Common.Position());
	//if (uthInput.Common.Event() == InputEvent::DRAG)
	//{
	//	p_arrow->update(p_caveman->transform.GetPosition());
	//}
	//if (uthInput.Mouse.IsButtonReleased(Mouse::MButton::LEFT) == true && p_arrow->IsActive())
	//{
	//	if (p_pauseButton->IsClicked() == false)
	//	{
	//		bool bigpull = p_arrow->IsStrong();
	//		p_caveman->ChangeDirectionMouse(p_arrow->GetNormDirection(), bigpull);
	//		if (bigpull)
	//		{
	//			stats.addOxygen -= 0.1f;
	//			p_cavemanMove->SetVolume(100); // OR SetPitch?
	//		}
	//		else
	//		{
	//			stats.addOxygen -= 0.05f;
	//			p_cavemanMove->SetVolume(40);
	//		}
	//		p_arrow->DisableArrow();
	//		p_cavemanMove->PlayEffect();
	//	}
	//	else p_arrow->DisableArrow();
	//}
	//if (uthInput.Mouse.IsButtonPressed(Mouse::MButton::RIGHT) == true && p_club->isHitting() == false)
	//{  
	//	p_clubAttack->PlayEffect();
	//	Vec2 hitPoint = uthEngine.GetWindow().PixelToCoords(uthInput.Common.Position());
	//	p_club->Hit(p_caveman->transform.GetPosition(), hitPoint);
	//	p_caveman->Hit(hitPoint, isDying);
	//}
}
void GameScene::UpdateButtonPositions()
{
	Vec2 offset = (p_pauseButton->transform.GetSize() / 4);
	p_pauseButton->transform.SetPosition(
										 uthEngine.GetWindow().GetCamera().GetPosition().x +
														 uthEngine.GetWindowResolution().x *
																					  0.5f -
																				  offset.x ,
										 uthEngine.GetWindow().GetCamera().GetPosition().y -
														 uthEngine.GetWindowResolution().y *
																					  0.5f + 
																				  offset.y);
}
void GameScene::GameOverLogic()
{
	// count and save final score
	string newscore = to_string(stats.GetFinalScore());
	FileManager fm;
	fm.OpenFile("newscore.dat", FileManager::Location::INTERNAL, true);
	fm.WriteString(newscore);
	fm.CloseFile();
	// go to end scene
	uthSceneM.GoToScene(3);
}
void GameScene::GameState()
{
	if (stats.GetGameTime() > 120)
	{
		asteroidAmount = 40;
		astronautAmount = 1;
		cosmonautAmount = 3;
	}
	else if (stats.GetGameTime() > 110)
	{
		asteroidAmount = 80;
		astronautAmount = 1;
		cosmonautAmount = 4;
	}
	else if (stats.GetGameTime() > 100)
	{
		asteroidAmount = 90;
		astronautAmount = 1;
		cosmonautAmount = 5;
	}
	else if (stats.GetGameTime() > 90)
	{
		asteroidAmount = 80;
		astronautAmount = 2;
		cosmonautAmount = 6;
	}
	else if (stats.GetGameTime() > 80)
	{
		asteroidAmount = 70;
		astronautAmount = 3;
		cosmonautAmount = 7;
	}
	else if (stats.GetGameTime() > 70)
	{
		asteroidAmount = 60;
		astronautAmount = 4;
		cosmonautAmount = 8;
	}
	else if (stats.GetGameTime() > 60)
	{
		asteroidAmount = 55;
		astronautAmount = 5;
		cosmonautAmount = 9;
	}
	else if (stats.GetGameTime() > 50)
	{
		asteroidAmount = 50;
		astronautAmount = 10;
		cosmonautAmount = 10;
	}
	else if (stats.GetGameTime() > 40)
	{
		asteroidAmount = 50;
		astronautAmount = 15;
		cosmonautAmount = 15;
	}
	else if (stats.GetGameTime() > 30)
	{
		asteroidAmount = 50;
		astronautAmount = 20;
		cosmonautAmount = 10;
	}
	else if (stats.GetGameTime() > 20)
	{
		asteroidAmount = 50;
		astronautAmount = 20;
		cosmonautAmount = 5;
	}
	else if (stats.GetGameTime() > 10)
	{
		asteroidAmount = 45;
		astronautAmount = 25;
		cosmonautAmount = 1;
	}
	else
	{
		asteroidAmount = 30;
		astronautAmount = 30;
		cosmonautAmount = 0;
	}
}

// Initialization

void GameScene::BackgroundInit()
{
	for (int i = 0; i < 4; ++i)
	{
		p_background[i] = new GameObject("Background");
		p_background[i]->AddComponent(new Sprite(uthRS.LoadTexture("Big_Background.png")));
	}
	p_background[1]->transform.SetPosition(Vec2(p_background[1]->GetComponent<Sprite>()->GetSize().x, 0));
	p_background[2]->transform.SetPosition(Vec2(0, p_background[1]->GetComponent<Sprite>()->GetSize().y));
	p_background[3]->transform.SetPosition(p_background[1]->GetComponent<Sprite>()->GetSize());
}
void GameScene::PauseInit()
{
	// Game paused overlay
	overlay = new GameObject();
	overlay->AddComponent(new Sprite(uthRS.LoadTexture("Overlay.png")));
	overlay->transform.ScaleToSize(uthEngine.GetWindow().GetCamera().GetSize());
	overlay->SetActive(false);
}
void GameScene::ButtonsInit()
{
	p_pauseButton = new Button(uthEngine.GetWindow(), uthRS.LoadTexture("Placeholders/Pause.png"));
	p_pauseButton->transform.ScaleToSize(p_pauseButton->transform.GetSize() / 2);
	p_pauseButton->SetActive(true);
	p_pauseButton->setCallBack([this]()
	{
		paused = true;
		Vec2 campos = uthEngine.GetWindow().GetCamera().GetPosition();
		float offsetY = uthEngine.GetWindow().GetCamera().GetSize().y / 4;
		p_playButton->SetActive(true);
		p_playButton->transform.SetPosition(campos.x, campos.y - offsetY);
		p_replayButton->SetActive(true);
		p_replayButton->transform.SetPosition(campos.x, campos.y + offsetY);
		overlay->SetActive(true);
		overlay->transform.SetPosition(campos);
	});

	p_playButton = new Button(uthEngine.GetWindow(), uthRS.LoadTexture("Placeholders/Play.png"));
	p_playButton->SetActive(false);
	p_playButton->setCallBack([this]()
	{
		paused = false;
		p_playButton->SetActive(false);
		p_replayButton->SetActive(false);
		overlay->SetActive(false);
	});

	p_replayButton = new Button(uthEngine.GetWindow(), uthRS.LoadTexture("Placeholders/Replay.png"));
	p_replayButton->SetActive(false);
	p_replayButton->setCallBack([this]()
	{
		uthSceneM.GoToScene(1);
	});
}
void GameScene::ContactLogicInit()
{
	contactListener = PhysicsContactListener();
	contactListener.onBeginContact = [this](b2Contact* contact, GameObject* a, GameObject* b)
	{
		if (!isDying)
		{
			if (b->HasTag("Club"))
			{
				ReactToHit(a);
				p_club->HasHit();
			}
			else if (a->HasTag("Club"))
			{
				ReactToHit(b);
				p_club->HasHit();
			}
		}
	};
	p_world->SetContactListener(&contactListener);
}
void GameScene::ParticleInit()
{
	// particle effect for astronaut kill

	p_partsys = new ParticleSystem(100);
	p_rockpartsys = new ParticleSystem(50);
	auto oxypart = uthRS.LoadTexture("Placeholders/oxypart.png");
	auto rockpart = uthRS.LoadTexture("Placeholders/rockpart.png");


	ParticleTemplate pt;
	pt.SetTexture(oxypart);
	pt.SetLifetime(2.5f);
	pt.SetSpeed(30.f, 70.f);
	pt.SetColor(1, 1, 1, 1);

	p_partsys->SetTemplate(pt);
	p_partsys->AddAffector(new OxygenAffector());
	p_partsys->SetEmitProperties(false);
	particleTimer = 0;

	ParticleTemplate pt_r;
	pt_r.SetTexture(rockpart);
	pt_r.SetLifetime(2.5f);
	pt_r.SetSpeed(70.f, 100.f);
	pt_r.SetColor(1, 1, 1, 1);

	p_rockpartsys->SetTemplate(pt_r);
	p_rockpartsys->AddAffector(new OxygenAffector());
	p_partsys->SetEmitProperties(false);
}
void GameScene::VariableInit()
{
	paused = false;
	screenDiameter = sqrt(pow(uthEngine.GetWindowResolution().x, 2) + pow(uthEngine.GetWindowResolution().y, 2));


	Randomizer::SetSeed(time(NULL));
	p_world = new PhysicsWorld(0, 0);

	p_caveman = new Caveman();
	AddChild<Caveman>(p_caveman);
	p_caveman->Init(p_world);

	p_club = new Club(p_caveman->GetComponent<Rigidbody>()->GetSize());
	p_club->Init(p_world);
	AddChild<Club>(p_club);

	p_arrow = new Arrow();
	p_arrow->Init();
	AddChild<Arrow>(p_arrow);
	deathTimer = 1.6f;
	isDying = false;

	//prefabObject = PrefabObject();
	//stats = Statistics();
	nauts = 0;
	uthEngine.GetWindow().GetCamera().SetPosition(p_caveman->transform.GetPosition());

	deleteAsteroid = nullptr;
}
void GameScene::SoundInit()
{
	music = uthRS.LoadSound("sounds/gamemusic.ogg");
	music->Loop(true);
	music->Play();

	p_astroDie = uthRS.LoadSound("sounds/astronaut_dying_groan.wav");
	p_astroHit = uthRS.LoadSound("sounds/astronaut_hit.wav");
	//p_panic1 = uthRS.LoadSound("sounds/astronaut_panic_voice_1.wav");
	//p_panic2 = uthRS.LoadSound("sounds/astronaut_panic_voice_2.wav");
	p_clubAttack = uthRS.LoadSound("sounds/caveman_club_attack.wav");
	p_cavemanMove = uthRS.LoadSound("sounds/caveman_move.wav");
	p_hitMetal = uthRS.LoadSound("sounds/hit_sound_metal.wav");
	p_hitRock = uthRS.LoadSound("sounds/hit_sound_rock.wav");
}
