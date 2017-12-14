//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#include "GameScreen_RainbowIslands.h"
#include <iostream>
#include "LevelMap.h"
#include "Character.h"
#include "CharacterBub.h"
#include "CharacterCaterpillar.h"
#include "CharacterRainbow.h"
#include "CharacterFruit.h"
#include "CharacterChest.h"
#include "Commons_RainbowIslands.h"
#include "Constants_RainbowIslands.h"
#include "../Texture2D.h"
#include "../Collisions.h"
#include "VirtualJoypad.h"

using namespace::std;

int RainbowOffsets[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,22,22,23,23,
						 23,22,22,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0, };
//--------------------------------------------------------------------------------------------------

GameScreen_RainbowIslands::GameScreen_RainbowIslands(SDL_Renderer* renderer)
	: GameScreen(renderer)
	, mGeneticAlgorithm(nullptr)
	, mTimeElapsedLastRunNN(0)
	, mTimeElapsedLevel(0)
	, mMaxPositionYCharacter(0.0f)
	, mCurrentNN(0)
	, mLastBubCharacterPosition(Vector2D(0.0f, 0.0f))
{
	srand(NULL);
	mLevelMap = NULL;
	SetUpLevel();

	for (int i = 0; i < kPopulationSize; i++)
	{
		///Neural Network
		NeuralNetworkPtr neuralNetwork(new NeuralNetwork(kNumberOfInputs, kNumberOfNeuronsPerHiddenLayer, kNumberOfOutputs));
		neuralNetwork->Initialise();
		mNeuralNetworks.push_back(neuralNetwork);
	}

	mGeneticAlgorithm.reset(new GeneticAlgorithm());
}

//--------------------------------------------------------------------------------------------------

GameScreen_RainbowIslands::~GameScreen_RainbowIslands()
{
	//Background image.
	delete mBackgroundTexture;
	mBackgroundTexture = NULL;

	//Player character.
	delete mBubCharacter;
	mBubCharacter = NULL;

	//Level map.
	delete mLevelMap;
	mLevelMap = NULL;

	//Treasure chest.
	delete mChest;
	mChest = NULL;

	//Enemies.
	mEnemies.clear();

	//Fruit.
	mFruit.clear();

	//Rainbows.
	mRainbows.clear();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::Render()
{
	//Draw the background.
	RenderBackground();

	//Draw the chest.
	if (mChest != NULL)
		mChest->Render();

	//Draw the Fruit.
	for (unsigned int i = 0; i < mFruit.size(); i++)
	{
		mFruit[i]->Render();
	}

	//Draw the Enemies.
	for(unsigned int i = 0; i < mEnemies.size(); i++)
	{
		mEnemies[i]->Render();
		DrawDebugCircle(mEnemies[i]->GetCentralPosition(), mEnemies[i]->GetCollisionRadius(), 255, 0, 0);
	}

	//Draw the Rainbows.
	for (unsigned int i = 0; i < mRainbows.size(); i++)
	{
		mRainbows[i]->Render();
		if(mRainbows[i]->CanKill())
			DrawDebugCircle(mRainbows[i]->GetStrikePosition(), mRainbows[i]->GetCollisionRadius(), 255, 255, 255);
	}

	//Draw the player.
	mBubCharacter->Render();
	DrawDebugCircle(mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius(), 0, 255, 0);

	DrawDebugInputs();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::RenderBackground()
{
	if (mLevelMap != NULL)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			for (int y = 0; y < MAP_HEIGHT; y++)
			{
				//Get the portion of the spritesheet you want to draw.
				int num = mLevelMap->GetBackgroundTileAt(y, x);
				int w = TILE_WIDTH*(num % kTileSpriteSheetWidth);
				int h = TILE_HEIGHT*(num / kTileSpriteSheetWidth);
				//								{XPos, YPos, WidthOfSingleSprite, HeightOfSingleSprite}
				SDL_Rect portionOfSpritesheet = { w, h, TILE_WIDTH, TILE_HEIGHT };

				//Determine where you want it drawn.
				SDL_Rect destRect = { x*TILE_WIDTH, kRainbowIslandsScreenHeight-((MAP_HEIGHT-y)*TILE_HEIGHT), TILE_WIDTH, TILE_HEIGHT };

				//Then draw it.
				mBackgroundTexture->Render(portionOfSpritesheet, destRect, SDL_FLIP_NONE);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::Update(size_t deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Virtual Joypad.
	//--------------------------------------------------------------------------------------------------
	VirtualJoypad::Instance()->SetJoypadState(e);

	//--------------------------------------------------------------------------------------------------
	//Update the level time.
	//--------------------------------------------------------------------------------------------------
	mTimeToCompleteLevel -= deltaTime;
	if (mTimeToCompleteLevel < TRIGGER_ANGRY_TIME && !mTriggeredAnger)
	{
		MakeEnemiesAngry();
		if (mTimeToCompleteLevel <= 0)
			RestartLevel();
	}

	//--------------------------------------------------------------------------------------------------
	//Update the player.
	//--------------------------------------------------------------------------------------------------
	mBubCharacter->Update(deltaTime, e);
	if (!mBubCharacter->GetAlive())
		RestartLevel();
	else if (mBubCharacter->GetCentralPosition().y < Y_POSITION_TO_COMPLETE)
		CreateChest(Vector2D(kRainbowIslandsScreenWidth*0.25f, -50.0f));


	//--------------------------------------------------------------------------------------------------
	//Update the game objects.
	//--------------------------------------------------------------------------------------------------
	UpdateFruit(deltaTime, e);
	UpdateEnemies(deltaTime, e);
	UpdateRainbows(deltaTime, e);

	//################################################
	mTimeElapsedLastRunNN += deltaTime;
	mTimeElapsedLevel += deltaTime;

	if (mTimeElapsedLastRunNN >= 0)
	{
		/*Vector2D positionCharacter = mBubCharacter->GetPosition();
		Vector2D nearestFruit;
		Vector2D nearestEnemy;
		float    distanceToTop           = 0.0f;
		float    nearestDistanceToFruit  = 10000.0f;
		float    nearestDistanceToEnemy  = 10000.0f;

		for(auto fruit: mFruit)
		{
			Vector2D positionFruit = fruit->GetCentralPosition();
			float distance = positionFruit.Distance(positionCharacter);

			if (distance < nearestDistanceToFruit)
			{
				nearestDistanceToFruit = distance;
				nearestFruit = positionFruit;
			}
		}

		for (auto ennemy : mEnemies)
		{
			Vector2D positionEnemy = ennemy->GetCentralPosition();
			float distance = positionEnemy.Distance(positionCharacter);

			if (distance < nearestDistanceToEnemy)
			{
				nearestDistanceToEnemy = distance;
				nearestEnemy = positionEnemy;
			}
		}

		distanceToTop = positionCharacter.y / kRainbowIslandsScreenHeight;

		std::vector<float> inputs;

		nearestFruit.Normalize();
		nearestEnemy.Normalize();

		inputs.push_back(nearestFruit.x);
		inputs.push_back(nearestFruit.y);
		inputs.push_back(nearestEnemy.x);
		inputs.push_back(nearestEnemy.y);
		inputs.push_back(distanceToTop);*/

		//Get Environment data
		mInputs = GetCharacterEnvironment();

		//Get current position
		mInputs.push_back(mBubCharacter->GetCentralPosition().y);

		//Is the player currently stuck
		if (mLastBubCharacterPosition.x == mBubCharacter->GetCentralPosition().x &&
			mLastBubCharacterPosition.y == mBubCharacter->GetCentralPosition().y)
		{
			mInputs.push_back(1.0f);
		}
		else
		{
			mInputs.push_back(0.0f);
		}

		//Start neural network
		mNeuralNetworks[mCurrentNN]->SetInputs(mInputs);
		mNeuralNetworks[mCurrentNN]->Run();
		std::vector<float>& outputs = mNeuralNetworks[mCurrentNN]->GetOutputs();

		mMaxPositionYCharacter = mBubCharacter->GetCentralPosition().y;

		//Reset JoyPad
		mJoyPad.JoyPadUp = true;
		mJoyPad.JoyPadDown = true;
		mJoyPad.JoyPadLeft = true;
		mJoyPad.JoyPadRight = true;

		VirtualJoypad::Instance()->SetJoypadState(mJoyPad, false);

		//Update JoyPad
		mJoyPad.JoyPadUp = false;
		mJoyPad.JoyPadDown = false;
		mJoyPad.JoyPadLeft = false;
		mJoyPad.JoyPadRight = false;

		std::vector<float> targets = GetTargets(mInputs);
		outputs = mNeuralNetworks[mCurrentNN]->GetOutputs();
		mNeuralNetworks[mCurrentNN]->BackPropagation(targets, mInputs);
		//outputs = targets;

		if (outputs[0] >= 0.9f) mJoyPad.JoyPadUp    = true;
		if (outputs[1] >= 0.9f) mJoyPad.JoyPadDown  = true;
		if (outputs[2] >= 0.9f) mJoyPad.JoyPadLeft  = true;
		if (outputs[3] >= 0.9f) mJoyPad.JoyPadRight = true;

		mTimeElapsedLastRunNN = 0;
	}

	VirtualJoypad::Instance()->SetJoypadState(mJoyPad);

	//We have a chest, but is it open yet.
	if (mChest != NULL)
	{
		mChest->Update(deltaTime, e);
		if (mChest->IsChestOpen() && !mTriggeredChestSpawns)
		{
			TriggerChestSpawns();
		}
	}

	//--------------------------------------------------------------------------------------------------
	//Check if we need to create a new rainbow.
	//--------------------------------------------------------------------------------------------------
	if (VirtualJoypad::Instance()->DownArrow && mCanSpawnRainbow)
	{
		bool collidingWithRainbow = false;
		
		for (unsigned int i = 0; i < mRainbows.size(); i++)
		{
			if (Collisions::Instance()->Box(mBubCharacter->GetCollisionBox(), mRainbows[i]->GetCollisionBox()))
			{
				collidingWithRainbow = true;
				break;
			}
		}

		if (!collidingWithRainbow)
		{
			Vector2D pos = mBubCharacter->GetPosition();
			pos.x += 10;
			if (mBubCharacter->GetFacing() == FACING_RIGHT)
				pos.x += mBubCharacter->GetCollisionBox().width-10;
			pos.y -= mBubCharacter->GetCollisionBox().height*0.3f;
			CreateRainbow(pos, mBubCharacter->GetRainbowsAllowed());

			mCanSpawnRainbow = false;
		}
	}
	else if (!VirtualJoypad::Instance()->DownArrow)
	{
		mCanSpawnRainbow = true;
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::MakeEnemiesAngry()
{
	if (!mEnemies.empty())
	{
		int enemyIndexToDelete = -1;
		for (unsigned int i = 0; i < mEnemies.size(); i++)
		{
			mEnemies[i]->MakeAngry();
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateEnemies(size_t deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Enemies.
	//--------------------------------------------------------------------------------------------------
	if(!mEnemies.empty())
	{
		int enemyIndexToDelete = -1;
		for(unsigned int i = 0; i < mEnemies.size(); i++)
		{
			mEnemies[i]->Update(deltaTime, e);

			//Check to see if the enemy collides with the player.
			if (Collisions::Instance()->Circle(mEnemies[i]->GetCentralPosition(), mEnemies[i]->GetCollisionRadius(), mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius()))
			{
				mBubCharacter->SetState(CHARACTERSTATE_PLAYER_DEATH);
			}

			//If the enemy is no longer alive, then schedule it for deletion.
			if(!mEnemies[i]->GetAlive())
			{
				enemyIndexToDelete = i;
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead enemies - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if(enemyIndexToDelete != -1)
		{
			Character* toDelete = mEnemies[enemyIndexToDelete];

			//We need a pickup to be generated.
			CreateFruit(toDelete->GetPosition(), true);

			mEnemies.erase(mEnemies.begin()+enemyIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateFruit(size_t deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Fruit.
	//--------------------------------------------------------------------------------------------------
	if(!mFruit.empty())
	{
		int fruitIndexToDelete = -1;
		for(unsigned int i = 0; i < mFruit.size(); i++)
		{
			//Update the fruit.
			mFruit[i]->Update(deltaTime, e);

			//check if the player has collided with it.
			if (Collisions::Instance()->Circle(mFruit[i]->GetCentralPosition(), mFruit[i]->GetCollisionRadius(), mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius()))
			{
				mFruit[i]->SetAlive(false);
				fruitIndexToDelete = i;
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead fruit - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if(fruitIndexToDelete != -1)
		{
			mBubCharacter->AddPoints();

			Character* toDelete = mFruit[fruitIndexToDelete];
			mFruit.erase(mFruit.begin() + fruitIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateRainbows(size_t deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Rainbows.
	//--------------------------------------------------------------------------------------------------
	
	//Need to turn flag off each frame incase the rainbow disapated.
	mBubCharacter->SetOnARainbow(false);

	if (!mRainbows.empty())
	{
		int rainbowIndexToDelete = -1;

		for (unsigned int i = 0; i < mRainbows.size(); i++)
		{
			int xPosition = (int)mBubCharacter->GetPosition().x + (int)(mBubCharacter->GetCollisionBox().width*0.5f);
			int footPosition = (int)(mBubCharacter->GetPosition().y + mBubCharacter->GetCollisionBox().height);

			//Update the rainbow.
			mRainbows[i]->Update(deltaTime, e);

			if (!mRainbows[i]->GetAlive())
				rainbowIndexToDelete = i;
			else
			{
				//check if the player has collided with it.
				if (!mBubCharacter->IsJumping())
				{
					if (Collisions::Instance()->PointInBox(Vector2D(xPosition, footPosition), mRainbows[i]->GetCollisionBox()))
					{
						mBubCharacter->SetState(CHARACTERSTATE_WALK);
						mBubCharacter->SetOnARainbow(true);
						int xPointOfCollision = (int)(mRainbows[i]->GetPosition().x + mRainbows[i]->GetCollisionBox().width - xPosition);
						if (mBubCharacter->GetFacing() == FACING_RIGHT)
							xPointOfCollision = (int)(xPosition - mRainbows[i]->GetPosition().x);

						//We don't want to pop between walking on different rainbows. Ensure the switch between rainbows looks 'realistic'
						double distanceBetweenPoints = footPosition - (mRainbows[i]->GetPosition().y - RainbowOffsets[xPointOfCollision]);
						if(distanceBetweenPoints < 40.0)
							mBubCharacter->SetPosition(Vector2D(mBubCharacter->GetPosition().x, mRainbows[i]->GetPosition().y - RainbowOffsets[xPointOfCollision]));
					}
				}

				//Check for collisions with enemies.
				for (unsigned int j = 0; j < mEnemies.size(); j++)
				{
					if (mRainbows[i]->CanKill())
					{
						if (Collisions::Instance()->Circle(mRainbows[i]->GetStrikePosition(), mRainbows[i]->GetCollisionRadius(), mEnemies[j]->GetPosition(), mEnemies[j]->GetCollisionRadius()))
						{
							mEnemies[j]->SetAlive(false);
						}
					}
				}
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead rainbow - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if (rainbowIndexToDelete != -1)
		{
			Character* toDelete = mRainbows[rainbowIndexToDelete];
			mRainbows.erase(mRainbows.begin() + rainbowIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

bool GameScreen_RainbowIslands::SetUpLevel()
{
	//Load the background texture.
	mBackgroundTexture = new Texture2D(mRenderer);
	if( !mBackgroundTexture->LoadFromFile("Images/RainbowIslands/Tiles.png"))
	{
		cout << "Failed to load background texture!";
		return false;
	}

	//Create the level map.
	SetLevelMap();

	CreateStartingCharacters();

	mTimeToCompleteLevel = LEVEL_TIME;
	mTriggeredAnger		 = false;

	return true;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::RestartLevel()
{
	if (mBubCharacter->GetCentralPosition().y < 75)
	{
		cout << "######### Won !" << endl;
	}

	//Add genome
	GenomePtr genome(new Genome());
	float y = mBubCharacter->GetPosition().y;
	float fitness =
		mBubCharacter->GetPoints() * 0.8f
		+ 1 / (kRainbowIslandsScreenHeight - mMaxPositionYCharacter) * 1.0f
		+ (mTimeElapsedLevel / 1000) * 0.2f;

	genome->SetWeights(mNeuralNetworks[mCurrentNN]->GetWeights());
	genome->SetFitness(fitness);
	mGeneticAlgorithm->AddGenome(genome);

	cout << "current character : " << mCurrentNN << "/" << fitness << "/" << mMaxPositionYCharacter << endl;

	mTimeElapsedLevel = 0;
	mMaxPositionYCharacter = 0.0f;

	if (mCurrentNN == kPopulationSize - 1)
	{
		cout << "evolution" << endl;

		mCurrentNN = 0;
		mGeneticAlgorithm->CalculateFitness();

		//Set Weights
		std::vector<GenomePtr> genomes = mGeneticAlgorithm->GetGenomes();
		int i = 0;

		for (auto genome : genomes)
		{
			mNeuralNetworks[i]->SetWeights(genome->GetWeights());
			i++;
		}

		mGeneticAlgorithm->Reset();
	}
	else
	{
		mCurrentNN++;
	}

	//Clean up current characters.
	//Player character.
	delete mBubCharacter;
	mBubCharacter = NULL;

	//Level map.
	delete mLevelMap;
	mLevelMap = NULL;

	//Enemies.
	mEnemies.clear();

	//Fruit.
	mFruit.clear();

	//Rainbows.
	mRainbows.clear();

	//Treasure chest.
	if (mChest != NULL)
	{
		delete mChest;
		mChest = NULL;
	}

	//Create the level map.
	SetLevelMap();

	//Respawn characters and map.
	CreateStartingCharacters();

	mTimeToCompleteLevel = LEVEL_TIME;
	mTriggeredAnger		 = false;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateStartingCharacters()
{
	//Set up the player character.
	mBubCharacter = new CharacterBub(mRenderer, "Images/RainbowIslands/bub.png", mLevelMap, Vector2D(100, 570));
	mCanSpawnRainbow = true;

	//Set up the bad guys.
	CreateCaterpillar(Vector2D(150, 500), FACING_LEFT);
	CreateCaterpillar(Vector2D(130, 403), FACING_LEFT);
	CreateCaterpillar(Vector2D(270, 450), FACING_LEFT);
	CreateCaterpillar(Vector2D(30, 350), FACING_RIGHT);
	CreateCaterpillar(Vector2D(30, 160), FACING_RIGHT);
	CreateCaterpillar(Vector2D(270, 160), FACING_LEFT);
	CreateCaterpillar(Vector2D(250, 227), FACING_RIGHT);

	//Setup starting pickups.
	CreateFruit(Vector2D(26, 575), false);
	CreateFruit(Vector2D(46, 575), false);
	CreateFruit(Vector2D(256, 575), false);
	CreateFruit(Vector2D(276, 575), false);
	CreateFruit(Vector2D(26, 544), false);
	CreateFruit(Vector2D(256, 544), false);
	CreateFruit(Vector2D(300, 544), false);
	CreateFruit(Vector2D(100, 496), false);
	CreateFruit(Vector2D(200, 496), false);
	CreateFruit(Vector2D(256, 450), false);
	CreateFruit(Vector2D(246, 227), false);
	CreateFruit(Vector2D(276, 227), false);
	CreateFruit(Vector2D(8, 160), false);
	CreateFruit(Vector2D(32, 160), false);
	CreateFruit(Vector2D(56, 160), false);
	CreateFruit(Vector2D(270, 160), false);
	CreateFruit(Vector2D(140, 177), false);
	CreateFruit(Vector2D(83, 112), false);
	CreateFruit(Vector2D(103, 112), false);
	CreateFruit(Vector2D(123, 112), false);

	mChest = NULL;
	mTriggeredChestSpawns = false;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::SetLevelMap()
{
	int backgroundMap[MAP_HEIGHT][MAP_WIDTH] = {
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,30,38,31},
	{7,7,7,7,8,5,5,6,7,7,7,7,7,7,7,8,5,5,5,5},
	{12,12,12,12,13,4,4,11,12,12,12,12,12,12,12,13,4,4,4,4},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,27,28,29,5,5,5,5,5,5,5,5,5,5,25,26,5,5,5},
	{5,5,32,33,34,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,25,26,5,5,5,5,5,5,5},
	{22,5,14,14,5,5,5,5,5,5,5,5,5,5,5,5,14,5,21,22},
	{4,4,4,4,4,5,14,14,14,14,5,5,5,5,5,5,4,4,4,4},
	{5,5,5,5,5,5,4,4,4,4,4,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,27,28,29,5,30,37,31},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,32,33,34,14,5,14,14},
	{4,4,4,5,5,14,14,25,26,5,5,5,5,5,5,4,4,4,4,4},
	{5,5,5,5,5,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,25,26,5,5,5,5,14,14,14,14,14,6,7,7,7,7,8,5,5},
	{5,5,5,5,4,4,4,4,4,4,4,4,11,12,12,12,12,13,4,4},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{4,4,4,5,5,5,19,5,19,5,25,26,5,5,5,5,5,5,5,25},
	{5,5,5,5,5,17,18,17,18,5,5,5,5,5,5,5,5,5,5,5},
	{21,22,21,22,5,15,16,15,16,5,5,5,5,19,5,5,5,30,36,31},
	{0,0,0,1,0,0,1,0,0,0,0,1,0,0,5,5,5,19,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0},
	{26,5,5,5,5,5,14,14,14,14,14,14,14,14,14,5,5,5,5,5},
	{5,5,5,19,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,19},
	{5,5,17,18,5,5,5,5,5,5,5,5,5,5,5,5,5,5,17,18},
	{14,14,15,16,5,5,5,5,5,5,5,5,27,28,29,5,14,14,15,16},
	{0,1,0,1,5,5,25,26,5,5,5,5,32,33,34,5,1,0,1,0},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{28,29,5,5,5,14,14,5,5,14,14,5,5,14,14,5,5,5,5,25},
	{33,34,19,5,5,0,0,1,0,0,0,0,1,0,0,5,5,5,5,5},
	{5,17,18,5,5,5,5,5,5,5,5,5,5,5,5,5,5,30,35,31},
	{14,15,16,5,27,28,29,5,19,5,5,5,19,5,5,5,5,5,5,5},
	{2,1,0,0,32,33,34,17,18,5,5,17,18,5,5,5,0,0,1,23},
	{24,24,24,9,9,9,5,15,16,5,5,15,16,5,9,9,9,24,24,24},
	{2,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,23},
	{20,20,20,20,20,20,10,10,10,20,20,10,10,10,20,20,20,20,20,20}};

	int collisionMap[MAP_HEIGHT][MAP_WIDTH] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1 },
		{ 0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 } };

	//Clear up any old map.
	if(mLevelMap != NULL)
	{
		delete mLevelMap;
	}

	//Set the new one.
	mLevelMap = new LevelMap(collisionMap, backgroundMap);
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateCaterpillar(Vector2D position, FACING direction)
{
	CharacterCaterpillar* caterpillarCharacter = new CharacterCaterpillar(mRenderer, "Images/RainbowIslands/Caterpillar.png", mLevelMap, position, direction);
	Character* tempCharacter = (Character*)caterpillarCharacter;
	mEnemies.push_back(tempCharacter);
	caterpillarCharacter = NULL;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateRainbow(Vector2D position, int numberOfRainbows)
{
	double xOffset   = 0.0;
	float  spwnDelay = 0.0f;

	do
	{
		if(mBubCharacter->GetFacing() == FACING_LEFT)
			position.x -= xOffset;
		else
			position.x += xOffset;
		
		CharacterRainbow* rainbowCharacter = new CharacterRainbow(mRenderer, "Images/RainbowIslands/Rainbow.png", position, mBubCharacter->GetFacing(), spwnDelay);
		if (rainbowCharacter->GetFacing() == FACING_LEFT)
			rainbowCharacter->SetPosition(Vector2D(rainbowCharacter->GetPosition().x - rainbowCharacter->GetCollisionBox().width, rainbowCharacter->GetPosition().y));
		
		mRainbows.push_back(rainbowCharacter);
		numberOfRainbows--;
		xOffset = rainbowCharacter->GetCollisionBox().width-5.0;
		spwnDelay += 200;
	} while (numberOfRainbows > 0);
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateFruit(Vector2D position, bool bounce)
{
	CharacterFruit* fruitCharacter = new CharacterFruit(mRenderer, "Images/RainbowIslands/Pickups.png", mLevelMap, position, (FRUIT_TYPE)(rand()%FRUIT_MAX));
	mFruit.push_back(fruitCharacter);

	if (bounce)
		fruitCharacter->Jump();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateChest(Vector2D position)
{
	if (mChest == NULL)
	{
		mChest = new CharacterChest(mRenderer, "Images/RainbowIslands/Chest.png", mLevelMap, position);
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::TriggerChestSpawns()
{
	Vector2D pos		= mChest->GetCentralPosition();
	int xRange			= (int)mChest->GetCollisionBox().width;
	int xDeductRange	= (int)mChest->GetCollisionBox().width/2;

CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);

mTriggeredChestSpawns = true;
}

//--------------------------------------------------------------------------------------------------

std::vector<float> GameScreen_RainbowIslands::GetCharacterEnvironment()
{
	std::vector<float> data;
	Vector2D characterPosition = mBubCharacter->GetCentralPosition();
	int characterPositionX = characterPosition.x / TILE_WIDTH;
	int characterPositionY = characterPosition.y / TILE_HEIGHT;
	Vector2D startSearchinPosition = characterPosition;

	//0 = nothing
	//1 = obstacle
	//2 = loot
	//3 = enemy
	//4 = rainbow
	//5 = chest

	for (int y = characterPositionY - 2; y <= characterPositionY + 2; y++)
	{
		for (int x = characterPositionX - 2; x <= characterPositionX + 2; x++)
		{
			int type = kInputNone;

			//We look for obstacle
			if (x > 0 && y > 0 && x < MAP_WIDTH)
			{
				type = mLevelMap->GetCollisionTileAt(y, x);
			}

			//Then we look for fruits
			for (auto fruit : mFruit)
			{
				Vector2D positionFruit = fruit->GetCentralPosition();
				float distance = positionFruit.Distance(Vector2D(x*TILE_WIDTH, y*TILE_HEIGHT));

				if (distance <= 12)
				{
					type = kInputCharacterFruit;
				}
			}

			//Then we look for rainbows
			for (auto rainbow : mRainbows)
			{
				Vector2D positionRainbow = rainbow->GetCentralPosition();
				float distance = positionRainbow.Distance(Vector2D(x*TILE_WIDTH, y*TILE_HEIGHT));

				if (distance <= 12)
				{
					type = kInputCharacterRainbow;
				}
			}

			//Then we look for ennemies
			for (auto enemy : mEnemies)
			{
				Vector2D positionEnemy = enemy->GetCentralPosition();
				float distance = positionEnemy.Distance(Vector2D(x*TILE_WIDTH, y*TILE_HEIGHT));

				if (distance <= 12)
				{
					type = kInputCharacterEnemy;
				}
			}

			//Then we look for chest
			/*if (mChest != nullptr)
			{
				Vector2D positionChest = mChest->GetCentralPosition();
				float distance = positionChest.Distance(Vector2D(x*TILE_WIDTH, y*TILE_HEIGHT));

				if (distance < TILE_WIDTH * 2)
				{
					type = kInputCharacterChest;
				}
			}*/

			data.push_back(type);
		}
	}

	return data;
}

std::vector<float> GameScreen_RainbowIslands::GetTargets(std::vector<float> inputs)
{
	std::vector<float> targets;
	FACING facing = mBubCharacter->GetFacing();

	for (int i = 0; i < kNumberOfOutputs; i++)
	{
		targets.push_back(0.0f);
	}

	bool onTop = false;

	if (inputs[25] < Y_POSITION_TO_COMPLETE + 10)
	{
		onTop = true;
	}

	//Rainbow below
	if (inputs[17] == kInputCharacterRainbow && !onTop)
	{
		//If enemy above on left
		if (inputs[7] == kInputCharacterEnemy || inputs[6] == kInputCharacterEnemy || inputs[5] == kInputCharacterEnemy)
		{
			targets[kOutputRight] = 1.0f;
			return targets;
		}

		//If enemy above on right
		if (inputs[7] == kInputCharacterEnemy || inputs[8] == kInputCharacterEnemy || inputs[9] == kInputCharacterEnemy  || inputs[9] == kInputCharacterEnemy)
		{
			targets[kOutputLeft] = 1.0f;
			return targets;
		}

		targets[kOutputUp] = 1.0f;

		if (facing == FACING::FACING_LEFT) targets[kOutputLeft] = 1.0f;
		else if (facing == FACING::FACING_RIGHT) targets[kOutputRight] = 1.0f;

		return targets;
	}

	//Enemy on right near
	if (inputs[13] == kInputCharacterEnemy)
	{
		targets[kOutputUp] = 1.0f;
		return targets;
	}

	//Enemy on left near
	if (inputs[11] == kInputCharacterEnemy)
	{
		targets[kOutputUp] = 1.0f;
		return targets;
	}

	//Enemy on right far
	if (inputs[14] == kInputObstacle && facing == FACING::FACING_RIGHT)
	{
		if (mCanSpawnRainbow && !onTop)
		{
			if(mInputs[26] != 1.0f) targets[kOutputDown] = 1.0f;
			return targets;
		}
	}

	//Enemy on left far
	if (inputs[10] == kInputObstacle && facing == FACING::FACING_LEFT)
	{
		if (mCanSpawnRainbow && !onTop && mInputs[26] != 1.0f)
		{
			if (mInputs[26] != 1.0f) targets[kOutputDown] = 1.0f;
			return targets;
		}
	}

	//Fruit on right
	if ((inputs[13] == kInputCharacterFruit || inputs[14] == kInputCharacterFruit) && facing == FACING::FACING_RIGHT)
	{
		targets[kOutputUp] = 1.0f;
		targets[kOutputRight] = 1.0f;
		return targets;
	}

	//Fruit on left
	if ((inputs[10] == kInputCharacterFruit || inputs[11] == kInputCharacterFruit) && facing == FACING::FACING_LEFT)
	{
		targets[kOutputUp] = 1.0f;
		targets[kOutputLeft] = 1.0f;
		return targets;
	}

	//Obstacle on right
	if (inputs[8] == kInputObstacle && facing == FACING::FACING_RIGHT)
	{
		if (mCanSpawnRainbow && !onTop)
		{
			if (mInputs[26] != 1.0f) targets[kOutputDown] = 1.0f;
			targets[kOutputUp] = 1.0f;
			targets[kOutputRight] = 1.0f;
			return targets;
		}
	}

	//Obstacle on left
	if (inputs[6] == kInputObstacle && facing == FACING::FACING_LEFT)
	{
		if (mCanSpawnRainbow && !onTop)
		{
			if (mInputs[26] != 1.0f) targets[kOutputDown] = 1.0f;
			targets[kOutputUp] = 1.0f;
			targets[kOutputLeft] = 1.0f;
			return targets;
		}
	}

	//Hole on right
	if (inputs[18] == kInputEmpty && facing == FACING::FACING_RIGHT)
	{
		if (mCanSpawnRainbow && !onTop)
		{
			if (mInputs[26] != 1.0f) targets[kOutputDown] = 1.0f;
			targets[kOutputUp] = 1.0f;
			targets[kOutputRight] = 1.0f;
			return targets;
		}
		else
		{
			targets[kOutputRight] = 1.0f;
			return targets;
		}
	}

	//Hole on left
	if (inputs[6] == kInputEmpty && facing == FACING::FACING_LEFT)
	{
		if (mCanSpawnRainbow && !onTop)
		{
			if (mInputs[26] != 1.0f) targets[kOutputDown] = 1.0f;
			targets[kOutputUp] = 1.0f;
			targets[kOutputLeft] = 1.0f;
			return targets;
		}
		else
		{
			targets[kOutputLeft] = 1.0f;
			return targets;
		}
	}

	//Nothing on right
	if (inputs[13] == kInputEmpty && facing == FACING::FACING_RIGHT)
	{
		targets[kOutputUp] = 1.0f;
		targets[kOutputRight] = 1.0f;
		return targets;
	}

	//Nothing on left
	if (inputs[11] == kInputEmpty && facing == FACING::FACING_LEFT)
	{
		targets[kOutputUp] = 1.0f;
		targets[kOutputLeft] = 1.0f;
		return targets;
	}

	//Outside on right
	if (inputs[13] == kInputNone)
	{
		targets[kOutputUp] = 1.0f;
		targets[kOutputLeft] = 1.0f;
		return targets;
	}

	//Outside on left
	if (inputs[11] == kInputNone)
	{
		targets[kOutputUp] = 1.0f;
		targets[kOutputRight] = 1.0f;
		return targets;
	}

	return targets;
}

void GameScreen_RainbowIslands::DrawDebugInputs()
{
	Vector2D characterPosition = mBubCharacter->GetCentralPosition();

	int index = 0;

	for (int y = -2; y <= 2; y++)
	{
		for (int x = -2; x <= 2; x++)
		{
			Vector2D position = characterPosition;
			position.x += TILE_WIDTH * x;
			position.y += TILE_HEIGHT * y;

			const int kInputNone = -1;
			const int kInputEmpty = 0;
			const int kInputObstacle = 1;
			const int kInputCharacterFruit = 2;
			const int kInputCharacterEnemy = 3;
			const int kInputCharacterRainbow = 4;
			const int kInputCharacterChest = 5;

			if(mInputs[index] == kInputNone)
				DrawDebugCircle(position, 8, 0, 0, 0);
			else if (mInputs[index] == kInputEmpty)
				DrawDebugCircle(position, 8, 255, 255, 255);
			else if (mInputs[index] == kInputObstacle)
				DrawDebugCircle(position, 8, 255, 165, 0);
			else if (mInputs[index] == kInputCharacterFruit)
				DrawDebugCircle(position, 8, 0, 255, 0);
			else if (mInputs[index] == kInputCharacterEnemy)
				DrawDebugCircle(position, 8, 255, 0, 0);
			else if (mInputs[index] == kInputCharacterRainbow)
				DrawDebugCircle(position, 8, 255, 215, 0);

			index++;
		}
	}
}