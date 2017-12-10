//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#include "GameScreen_Conway.h"
#include <stdlib.h>
#include "../Texture2D.h"
#include "ConwayConstants.h"
#include "../TinyXML/tinyxml.h"

//--------------------------------------------------------------------------------------------------

GameScreen_Conway::GameScreen_Conway(SDL_Renderer* renderer) : GameScreen(renderer)
{
	mMap = new int*[kConwayScreenWidth / kConwayTileDimensions];
	for (int i = 0; i < kConwayScreenWidth / kConwayTileDimensions; i++)
		mMap[i] = new int[kConwayScreenHeight / kConwayTileDimensions];

	mMapTemp = new int*[kConwayScreenWidth / kConwayTileDimensions];
	for (int i = 0; i < kConwayScreenWidth / kConwayTileDimensions; i++)
		mMapTemp[i] = new int[kConwayScreenHeight / kConwayTileDimensions];

	//Get all required textures.
	mWhiteTile = new Texture2D(renderer);
	mWhiteTile->LoadFromFile("Images/Conway/WhiteTile.png");
	mBlackTile = new Texture2D(renderer);
	mBlackTile->LoadFromFile("Images/Conway/BlackTile.png");

	//Start map has 50% active cells.
	CreateRandomMap(75);

	//Start game in frozen state.
	mPause = true;
}

//--------------------------------------------------------------------------------------------------

GameScreen_Conway::~GameScreen_Conway()
{
	delete mWhiteTile;
	mWhiteTile = NULL;

	delete mBlackTile;
	mBlackTile = NULL;

	for (int i = 0; i < kConwayScreenWidth / kConwayTileDimensions; i++)
	{
		delete mMap[i];
		mMap[i] = NULL;
	}
	delete mMap;
	mMap = NULL;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::Render()
{
	for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
	{
		for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
		{
			switch (mMap[x][y])
			{
			case 0:
				mBlackTile->Render(Vector2D(x*kConwayTileDimensions, y*kConwayTileDimensions));
				break;

			case 1:
				mWhiteTile->Render(Vector2D(x*kConwayTileDimensions, y*kConwayTileDimensions));
				break;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::Update(size_t deltaTime, SDL_Event e)
{
	//Additional input outside of player.
	switch (e.type)
	{
		//Deal with mouse click input.
	case SDL_KEYUP:
		switch (e.key.keysym.sym)
		{
		case SDLK_SPACE:
			mPause = !mPause;
			break;

		case SDLK_r:
			mPause = true;
			CreateRandomMap(75);
			break;

		case SDLK_l:
			mPause = true;
			LoadMap("Conway/ConwaySeed.xml");
			break;
		}
		break;

	default:
		break;
	}

	if (!mPause)
		UpdateMap();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::UpdateMap()
{
	int neighboursCounter = 0;

	for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
	{
		for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
		{
			neighboursCounter = countNeighbours(x, y);

			mMapTemp[x][y] = mMap[x][y] && (neighboursCounter == 2 || neighboursCounter == 3) || !mMapTemp[x][y] && neighboursCounter == 3;

			if (mMap[x][y] == 1)
			{
				if (neighboursCounter < 2)
				{
					mMapTemp[x][y] = 0;
				}
				else if (neighboursCounter <= 3)
				{
					mMapTemp[x][y] = 1;
				}
				else
				{
					mMapTemp[x][y] = 0;
				}
			}
			else if (neighboursCounter == 3)
			{
				mMapTemp[x][y] = 1;
			}
			else
			{
				mMapTemp[x][y] = 0;
			}
		}
	}

	for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
	{
		for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
		{
			mMap[x][y] = mMapTemp[x][y];
		}
	}
}

int GameScreen_Conway::countNeighbours(int x, int y)
{
	int neighboursCounter = 0;

	for (int j = -1; j <= 1; j++)
	{
		if (y + j < 0 || y + j >= kConwayScreenHeight / kConwayTileDimensions)
		{
			continue;
		}

		for (int i = -1; i <= 1; i++)
		{
			if (x + i < 0 || x + i >= kConwayScreenWidth / kConwayTileDimensions)
			{
				continue;
			}

			if (mMap[i + x][j + y] == 1 && (i != x || j != y))
			{
				neighboursCounter++;
			}
		}
	}

	if (mMap[x][y] == 1)
	{
		neighboursCounter--;
	}

	return neighboursCounter;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::CreateRandomMap(int percentageInactive)
{
	int randomNumber = 0;

	for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
	{
		for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
		{
			randomNumber = (rand() % 100) + 1;

			if (randomNumber > percentageInactive)
				mMap[x][y] = 1;
			else
				mMap[x][y] = 0;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::LoadMap(std::string path)
{
	//Get the whole xml document.
	TiXmlDocument doc;
	if (!doc.LoadFile(path))
	{
		cerr << doc.ErrorDesc() << endl;
	}

	//Now get the root element.
	TiXmlElement* root = doc.FirstChildElement();
	if (!root)
	{
		cerr << "Failed to load file: No root element." << endl;
		doc.Clear();
	}
	else
	{
		//Jump to the first 'objectgroup' element.
		for (TiXmlElement* groupElement = root->FirstChildElement("objectgroup"); groupElement != NULL; groupElement = groupElement->NextSiblingElement())
		{
			string name = groupElement->Attribute("name");
			if (name == "Seed")
			{
				int x = 0;
				int y = 0;

				//Jump to the first 'object' element - within 'objectgroup'
				for (TiXmlElement* objectElement = groupElement->FirstChildElement("object"); objectElement != NULL; objectElement = objectElement->NextSiblingElement())
				{
					string name = objectElement->Attribute("name");
					if (name == "TileTypes")
					{
						//Reset x position of map to 0 at start of each element.
						x = 0;

						//Split up the comma delimited connections.
						stringstream ss(objectElement->Attribute("value"));
						int i;
						while (ss >> i)
						{
							mMap[x][y] = i;

							if (ss.peek() == ',')
								ss.ignore();

							//Increment the x position.
							x++;
						}

						//Move down to the next y position in the map.
						y++;
					}
				}
			}
		}
	}
}