//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#include "GameScreen_Lunar.h"
#include <stdlib.h>
#include "../Texture2D.h"
#include "LunarConstants.h"
#include "LunarLander.h"
#include <time.h>
#include <fstream>

//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::GameScreen_Lunar(SDL_Renderer* renderer) 
	: GameScreen(renderer)
{
	srand(static_cast <unsigned> (time(0)));

	//Set up the platform to land on.
	mPlatform = new Texture2D(renderer);
	mPlatform->LoadFromFile("Images/Lunar/Platform.png");

	//Create a lunar lander to use for tests.
	mPlatformPosition = Vector2D(198,430);

	//Player controlled lander.
	mLunarLander = new LunarLander(renderer, Vector2D(400,125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));

	//AI controlled landers.
	mGeneration = 0;
	GenerateRandomChromosomes();
	for(int i = 0; i < kNumberOfAILanders; i++)
		mAILanders[i] = new LunarLander(renderer, Vector2D(400,125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));
	RestartGA();
	mAllowMutation = true;
	mAccumulatedDeltaTime = 0;
	mSuccess = 0;
	//-------------------------------------

	//Start game in frozen state.
	mPause = true;

	//Start game in player control.
	mAIOn = false;

}

//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::~GameScreen_Lunar()
{
	delete mPlatform;
	mPlatform = NULL;

	delete mLunarLander;
	mLunarLander = NULL;

	for(int i = 0; i < kNumberOfAILanders; i++)
		delete *mAILanders;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Render()
{
	mPlatform->Render(mPlatformPosition);

	if(mAIOn)
	{
		for(int i = 0; i < kNumberOfAILanders; i++)
			mAILanders[i]->Render();
	}
	else
		mLunarLander->Render();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Update(size_t deltaTime, SDL_Event e)
{
	//Additional input outside of player.
	switch(e.type)
	{
		//Deal with mouse click input.
		case SDL_KEYUP:
			switch(e.key.keysym.sym)
			{
				case SDLK_SPACE:
					mPause = !mPause;
					if(mPause)
						cout << "Paused" << endl;
					else
						cout << "Unpaused" << endl;
				break;

				case SDLK_a:
					mAIOn = !mAIOn;
					if(mAIOn)
						cout << "AI on" << endl;
					else
						cout << "AI off" << endl;
				break;

				case SDLK_LEFT:
					if(!mAIOn)
						mLunarLander->TurnLeft(deltaTime);
				break;

				case SDLK_RIGHT:
					if(!mAIOn)
						mLunarLander->TurnRight(deltaTime);
				break;

				case SDLK_UP:
					if(!mAIOn)
						mLunarLander->Thrust();
				break;

				case SDLK_r:
					if (!mAIOn)
					{
						mLunarLander->Reset();
						return;
					}
				break;
			}
		break;
	
		default:
		break;
	}
	
	if(!mPause)
	{
		mAccumulatedDeltaTime += deltaTime;
		if (mAccumulatedDeltaTime > 50)
		{
			//Reset the count.
			mAccumulatedDeltaTime = 0;

			if (!mAIOn)
				mLunarLander->Update(deltaTime, e);
			else
				UpdateAILanders(deltaTime, e);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::GenerateRandomChromosomes()
{
	for(int lander = 0; lander < kNumberOfAILanders; lander++)
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			mChromosomes[lander][action] = (LunarAction)(rand()%LunarAction_MaxActions);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::ClearChromosomes()
{
	for(int lander = 0; lander < kNumberOfAILanders; lander++)
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			mChromosomes[lander][action] = LunarAction_None;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::UpdateAILanders(size_t deltaTime, SDL_Event e)
{
	int numberDeadOrLanded = 0;

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		mAILanders[i]->Update(deltaTime, e);

		if (!mAILanders[i]->IsAlive() || mAILanders[i]->HasLanded())
			numberDeadOrLanded++;
	}

	//Are all finished?
	if (numberDeadOrLanded == kNumberOfAILanders)
	{
		CalculateFitness();
		return;
	}

	//Move on to the next action in the Chromosomes.
	mCurrentAction++;
	if (mCurrentAction >= kNumberOfChromosomeElements)
		return;

	//Assign all actions to landers.
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		switch (mChromosomes[i][mCurrentAction])
		{
			case LunarAction_LunarLeft:
				mAILanders[i]->TurnLeft(deltaTime);
			break;

			case LunarAction_LunarRight:
				mAILanders[i]->TurnRight(deltaTime);
			break;

			case LunarAction_LunarThrust:
				mAILanders[i]->Thrust();
			break;
		}
	}
}
//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::CalculateFitness()
{
	mMaxFitnessValue = 0.0f;

	float maxDistance = 0.0f;
	float maxTime = 0.0f;

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		float distance = mAILanders[i]->GetCentralPosition().Distance(mPlatformPosition);
		float time = mAILanders[i]->GetSurvivalTime();

		if (maxDistance < distance)
		{
			maxDistance = distance;
		}

		if (maxTime < time)
		{
			maxTime = time;
		}
	}

	//Compute fitness value
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		float distance = mAILanders[i]->GetCentralPosition().Distance(mPlatformPosition);
		float angle = mAILanders[i]->GetRotationAngle();
		float survivalTime = mAILanders[i]->GetSurvivalTime();
		float landingBonus = 0.0f;

		if (angle == 0.0f)
		{
			angle = 0.01f;
		}

		if (mAILanders[i]->HasLanded())
		{
			mSuccess++;
			landingBonus += kLandingBonus;
			mHasLandedChromosomes[i] = true;
		}
		else
		{
			mHasLandedChromosomes[i] = false;
		}

		distance = 1.0f - (distance / maxDistance);
		angle = 1.0f - (abs(angle) / 180.0f);
		survivalTime = survivalTime / maxTime;

		float fitness = (kDistWeight * (distance)) + (kRotWeight * (angle)) + (kAirTimeWeight * survivalTime) + (landingBonus);
		mFitnessValues[i] = fitness;
		mAILandersSorted[i] = i;

		cout << mFitnessValues[i] << endl;

		if (mMaxFitnessValue < mFitnessValues[i])
		{
			mMaxFitnessValue = mFitnessValues[i];
		}
	}

	//Sort all values
	for (int i = 1; i < kNumberOfAILanders; i++)
	{
		if (mFitnessValues[i] >= mFitnessValues[i - 1])
		{
			int tmpIndex            = mAILandersSorted[i];
			mAILandersSorted[i]     = mAILandersSorted[i - 1];
			mAILandersSorted[i - 1] = tmpIndex;
		}
	}

	cout << "Number of success landing " << mSuccess << endl;
	Selection();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Selection()
{
	mNumberSelectedAIChromosomes = 0;
	mNumberLanderChromosomes = 0;

	//Normalize fitness values
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		mFitnessValues[i] = mFitnessValues[i] / mMaxFitnessValue;
	}

	//Get all chromosomes which fintess superior to 0.6
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		if(mFitnessValues[i] >= 0.4f || mHasLandedChromosomes[i])
		{ 
			for (int j = 0; j < kNumberOfChromosomeElements; j++)
			{
				mSelectedAIChromosomes[mNumberSelectedAIChromosomes][j] = mChromosomes[i][j];
			}

			if (mHasLandedChromosomes[i])
			{
				mHasLandedSelectedChromosomes[mNumberSelectedAIChromosomes] = true; 
				mNumberLanderChromosomes++;
			}	
			else
			{
				mHasLandedSelectedChromosomes[mNumberSelectedAIChromosomes] = false;
			}

			mNumberSelectedAIChromosomes++;
		}
	}

	if (mNumberSelectedAIChromosomes < 2)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			if (mHasLandedChromosomes[0])
				mHasLandedSelectedChromosomes[1] = true;
			else
				mHasLandedSelectedChromosomes[1] = false;

			mSelectedAIChromosomes[1][j] =
				mSelectedAIChromosomes[0][j];
		}

		mNumberSelectedAIChromosomes++;
	}

	cout << "selected : " << mNumberSelectedAIChromosomes << endl;
	cout << "fitness  : " << mMaxFitnessValue << endl;
	cout << "landed   : " << mNumberLanderChromosomes << endl;

	Crossover();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Crossover()
{
	//Todo: Code this function.

	//How many elements we take from each parents
	int chunkSize = kNumberOfChromosomeElements / 4;

	//We select the two parents needed for crossover
	int parent[2];

	//Which parent should we use (0 = left, 1 = right)
	int whichParent = 0;

	for (int i = mNumberSelectedAIChromosomes; i < kNumberOfAILanders; i++)
	{
		parent[0] = (rand() % mNumberSelectedAIChromosomes);
		parent[1] = (rand() % mNumberSelectedAIChromosomes);

		if (rand() % (10000 / kCrossoverRate))
		{
			for (int j = 0; j < kNumberOfChromosomeElements; j++)
			{
				mSelectedAIChromosomes[i][j] = mSelectedAIChromosomes[parent[whichParent]][j];

				//We need to change parent
				if ((j + 1) % chunkSize == 0)
				{
					if (whichParent == 0) whichParent = 1;
					else whichParent = 0;
				}
			}

			mHasLandedSelectedChromosomes[i] = false;

			if(mNumberSelectedAIChromosomes < 100)
				mNumberSelectedAIChromosomes++;
		}
		else
		{
			int k = rand() % mNumberSelectedAIChromosomes;

			if (mHasLandedSelectedChromosomes[k])
			{
				mHasLandedSelectedChromosomes[i] = true;
			}
			else
			{
				mHasLandedSelectedChromosomes[i] = false;
			}

			for (int j = 0; j < kNumberOfChromosomeElements; j++)
			{
				mSelectedAIChromosomes[i][j] = mSelectedAIChromosomes[k][j];
			}
		}
	}
	
	Mutation();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Mutation()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			float mutationRate = 1.0f;

			if(mNumberLanderChromosomes > 0)
				mutationRate = ((float)mNumberLanderChromosomes / (float)kNumberOfAILanders);

			if (!mHasLandedSelectedChromosomes[i] && rand() % 10000 < (kMutationRate/mutationRate))
			{
				mSelectedAIChromosomes[i][j] = (LunarAction)(rand() % LunarAction_MaxActions);
			}

			mChromosomes[i][j] = mSelectedAIChromosomes[i][j];
		}
	}

	RestartGA();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::RestartGA()
{
	if(mGeneration >= 500)
	{
		GenerateRandomChromosomes();
		mGeneration = 0;
		mAllowMutation = true;
		cout << endl << "---FAILED (Restart)---" << endl << endl;
	}

	for(int i = 0; i < kNumberOfAILanders; i++)
	{
		mFitnessValues[i] = 0.0f;
		mAILanders[i]->Reset();
	}

	mAccumulatedDeltaTime = 0;
	mCurrentAction		  = 0;
	//mPause				  = true;
	//cout << "Paused" << endl;
	mGeneration++;
	cout << "---GENERATION " << mGeneration << "---" << endl;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::SaveSolution()
{
	//Output the solution of the first chromosome - They should all match at the point we output so it
	//shouldn't matter which chromosome is used for the solution.
	ofstream outFile("Solution.txt");
	if(outFile.is_open())
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			switch(mChromosomes[0][action])
			{
				case LunarAction_None:
					outFile << "0";
				break;
			
				case LunarAction_LunarThrust:
					outFile << "1";
				break;

				case LunarAction_LunarLeft:
					outFile << "2";
				break;

				case LunarAction_LunarRight:
					outFile << "3";
				break;
			}
		
			outFile << ",";
		}

		outFile << endl << "Generations taken: " << mGeneration;
		outFile << endl << "Platform start (" << mPlatformPosition.x << "," << mPlatformPosition.y << ")";
		outFile << endl << "Ship start (" << mAILanders[0]->GetStartPosition().x << "," << mAILanders[0]->GetStartPosition().y << ")";

		outFile.close();
	}
}

//--------------------------------------------------------------------------------------------------
