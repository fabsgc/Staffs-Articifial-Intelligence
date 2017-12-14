//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef _GAMESCREEN_RAINBOW_H   
#define _GAMESCREEN_RAINBOW_H

#include "../GameScreen.h"
#include "Commons_RainbowIslands.h"
#include "../Commons.h"
#include <SDL.h>
#include <vector>
#include "VirtualJoypad.h"

#include "NeuralNetwork.h"
#include "GeneticAlgorithm.h"

using namespace::std;

class Texture2D;
class Character;
class CharacterBub;
class CharacterFruit;
class CharacterRainbow;
class CharacterChest;
class LevelMap;


class GameScreen_RainbowIslands : GameScreen
{
//--------------------------------------------------------------------------------------------------
public:
	GameScreen_RainbowIslands(SDL_Renderer* renderer);
	~GameScreen_RainbowIslands();

	bool SetUpLevel();
	void Render();
	void RenderBackground();
	void Update(size_t deltaTime, SDL_Event e);

//--------------------------------------------------------------------------------------------------
private:
	void SetLevelMap();
	void RestartLevel();
	void CreateStartingCharacters();

	void MakeEnemiesAngry();
	void UpdateEnemies(size_t deltaTime, SDL_Event e);
	void CreateCaterpillar(Vector2D position, FACING direction);

	void UpdateRainbows(size_t deltaTime, SDL_Event e);
	void CreateRainbow(Vector2D position, int numberOfRainbows);

	void UpdateFruit(size_t deltaTime, SDL_Event e);
	void CreateFruit(Vector2D position, bool bounce);

	void CreateChest(Vector2D position);
	void TriggerChestSpawns();

	std::vector<float> GetCharacterEnvironment();
	std::vector<float> GetTargets(std::vector<float> inputs);

	void DrawDebugInputs();

//--------------------------------------------------------------------------------------------------
private:
	Texture2D*				      mBackgroundTexture;
	CharacterBub*			      mBubCharacter;
	bool					      mCanSpawnRainbow;
	vector<Character*>		      mEnemies;
	vector<CharacterFruit*>	      mFruit;
	vector<CharacterRainbow*>     mRainbows;
	CharacterChest*			      mChest;
	LevelMap*				      mLevelMap;

	size_t					      mTimeToCompleteLevel;
	bool					      mTriggeredAnger;

	bool					      mTriggeredChestSpawns;

	//Neural networks + Genetic algorithm
	std::vector<NeuralNetworkPtr> mNeuralNetworks;
	GeneticAlgorithmPtr           mGeneticAlgorithm;
	JoyPad                        mJoyPad;

	//States recording
	UINT                          mTimeElapsedLastRunNN;
	UINT                          mTimeElapsedLevel;
	float                         mMaxPositionYCharacter;

	//Index of the current character inside the neuralNetwork
	UINT                          mCurrentNN;

	std::vector<float>            mInputs;
	Vector2D                      mLastBubCharacterPosition;
};

#endif //_GAMESCREEN_RAINBOW_H