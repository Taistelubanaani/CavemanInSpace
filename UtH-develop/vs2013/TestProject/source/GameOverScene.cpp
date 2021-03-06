#include <GameOverScene.h>

GameOverScene::GameOverScene()
	: end()
{}
GameOverScene::~GameOverScene() {}

bool GameOverScene::Init()
{
	background = new GameObject();
	background->AddComponent(new Sprite(uthRS.LoadTexture("Placeholders/endBg.png")));
	background->transform.ScaleToSize(uthEngine.GetWindow().GetCamera().GetSize());
	background->transform.SetPosition(uthEngine.GetWindow().GetCamera().GetPosition());
	
	startTex = uthRS.LoadTexture("Placeholders/Replay.png");

	//music = uthRS.LoadSound("sounds/gamemusic.ogg");
	//music->Loop(true);
	//music->Play();

	// fill in the scores array so that newest is last
	FindOldScores();
	GetFinalScore();

	end.SetFinalScore(allscores[3]);
	int newScore = allscores[3]; // because SaveHighScores will change list order
	
	SaveHighScores();
	end.SetHighScores(allscores[0], allscores[1], allscores[2], newScore);

	startButton = new Button(uthEngine.GetWindow(), startTex);	
	startButton->setCallBack([]()
	{
		uthSceneM.GoToScene(2);
	});

	float offsetX = 3 * uthEngine.GetWindow().GetCamera().GetSize().x / 10;
	float offsetY = uthEngine.GetWindow().GetCamera().GetSize().y / 6;

	startButton->transform.SetPosition(uthEngine.GetWindow().GetCamera().GetPosition() + Vec2(offsetX, offsetY));
	AddChild<Button>(startButton);

	return true;
}
bool GameOverScene::DeInit()
{
	return true;
}
void GameOverScene::Update(float dt)
{
	Scene::Update(dt);
	end.Update(dt);
}
void GameOverScene::Draw(RenderTarget& target, RenderAttributes attributes)
{
	background->Draw(target, attributes);
	Scene::Draw(target, attributes);
	end.Draw(target, attributes);
}

// Private //

void GameOverScene::GetFinalScore()
{
	files.OpenFile("newscore.dat", FileManager::Location::INTERNAL);
	stringscores[3] = files.ReadText();
	files.CloseFile();
	allscores[3] = stoi(stringscores[3]);
}

void GameOverScene::FindOldScores()
{
	// read higscores

	files.OpenFile("highscores1.dat", FileManager::Location::INTERNAL);
	stringscores[0] = files.ReadText();
	files.CloseFile();
	files.OpenFile("highscores2.dat", FileManager::Location::INTERNAL);
	stringscores[1] = files.ReadText();
	files.CloseFile();
	files.OpenFile("highscores3.dat", FileManager::Location::INTERNAL);
	stringscores[2] = files.ReadText();
	files.CloseFile();

	for (int i = 0; i < 3; ++i)
	{
		allscores[i] = stoi(stringscores[i]);
	}
}

void GameOverScene::SaveHighScores()
{
	// sort scores
	int temp;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 3; i > j; --i)
		{
			if (allscores[i] > allscores[i - 1])
			{
				temp = allscores[i];
				allscores[i] = allscores[i - 1];
				allscores[i - 1] = temp;				
			}
		}
	}
	for (int i = 0; i < 3; ++i)
	{
		stringscores[i] = to_string(allscores[i]);
	}

	// write new highscore list, 3 biggest scores

	files.OpenFile("highscores1.dat", FileManager::Location::INTERNAL, true);
	files.WriteString(stringscores[0]);
	files.CloseFile();
	files.OpenFile("highscores2.dat", FileManager::Location::INTERNAL, true);
	files.WriteString(stringscores[1]);
	files.CloseFile();
	files.OpenFile("highscores3.dat", FileManager::Location::INTERNAL, true);
	files.WriteString(stringscores[2]);
	files.CloseFile();
}