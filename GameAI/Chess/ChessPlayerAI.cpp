//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#include "ChessPlayerAI.h"
#include <iostream>		//cout
#include <SDL.h>
#include <iomanip>		//Precision
#include <algorithm>	//Sort
#include "ChessConstants.h"
#include "../Texture2D.h"
#include "ChessMoveManager.h"

using namespace::std;

//--------------------------------------------------------------------------------------------------

const int kPawnScore = 1;
const int kKnightScore = 5;
const int kBishopScore = 5;
const int kRookScore = 10;
const int kQueenScore = 25;
const int kKingScore = 0;

const int kCheckScore		= 1;
const int kCheckmateScore	= 1;
const int kStalemateScore	= 1;	//Tricky one because sometimes you want this, sometimes you don't.

const int kPieceWeight		= 1; //Scores as above.
const int kMoveWeight		= 1; //Number of moves available to pieces.
const int kPositionalWeight	= 1; //Whether in CHECK, CHECKMATE or STALEMATE.

//--------------------------------------------------------------------------------------------------

ChessPlayerAI::ChessPlayerAI(SDL_Renderer* renderer, COLOUR colour, Board* board, vector<Vector2D>* highlights, Vector2D* selectedPiecePosition, Move* lastMove, int* searchDepth)
	: ChessPlayer(renderer, colour, board, highlights, selectedPiecePosition, lastMove)
{
	mDepthToSearch = searchDepth;
}

//--------------------------------------------------------------------------------------------------

ChessPlayerAI::~ChessPlayerAI()
{
}

//--------------------------------------------------------------------------------------------------

bool ChessPlayerAI::MakeAMove(Board* board, Move* move)
{	
	//If this was an en'passant move the taken piece will not be in the square we moved to.
	if (board->currentLayout[move->from_X][move->from_Y].piece == PIECE_PAWN)
	{
		//If the pawn is on its start position and it double jumps, then en'passant may be available for opponent.
		if ((move->from_Y == 1 && move->to_Y == 3) ||
			(move->from_Y == 6 && move->to_Y == 4))
		{
			board->currentLayout[move->from_X][move->from_Y].canEnPassant = true;
		}
	}

	//En'Passant removal of enemy pawn.
	//If our pawn moved into an empty position to the left or right, then must be En'Passant.
	if (board->currentLayout[move->from_X][move->from_Y].piece == PIECE_PAWN &&
		board->currentLayout[move->to_X][move->to_Y].piece == PIECE_NONE)
	{
		int pawnDirectionOpposite = mTeamColour == COLOUR_WHITE ? -1 : 1;

		if ((move->from_X < move->to_X) ||
			(move->from_X > move->to_X))
		{
			board->currentLayout[move->to_X][move->to_Y + pawnDirectionOpposite] = BoardPiece();
		}
	}

	//CASTLING - Move the rook.
	if (board->currentLayout[move->from_X][move->from_Y].piece == PIECE_KING)
	{
		//Are we moving 2 spaces??? This indicates CASTLING.
		if (move->to_X - move->from_X == 2)
		{
			//Moving 2 spaces to the right - Move the ROOK on the right into its new position.
			board->currentLayout[move->from_X + 3][move->from_Y].hasMoved = true;
			board->currentLayout[move->from_X + 1][move->from_Y] = board->currentLayout[move->from_X + 3][move->from_Y];
			board->currentLayout[move->from_X + 3][move->from_Y] = BoardPiece();
		}
		else if (move->to_X - move->from_X == -2)
		{
			//Moving 2 spaces to the left - Move the ROOK on the left into its new position.
			//Move the piece into new position.
			board->currentLayout[move->from_X - 4][move->from_Y].hasMoved = true;
			board->currentLayout[move->from_X - 1][move->from_Y] = board->currentLayout[move->from_X - 4][move->from_Y];
			board->currentLayout[move->from_X - 4][move->from_Y] = BoardPiece();
		}
	}

	//Move the piece into new position.
	board->currentLayout[move->from_X][move->from_Y].hasMoved = true;
	board->currentLayout[move->to_X][move->to_Y] = board->currentLayout[move->from_X][move->from_Y];
	board->currentLayout[move->from_X][move->from_Y] = BoardPiece();

	//If we operate the move on the real chess board (not from the minimax algorithm)
	if (board == mChessBoard)
	{
		//Store the last move to output at start of turn.
		mLastMove->from_X = move->from_X;
		mLastMove->from_Y = move->from_Y;
		mLastMove->to_X = move->to_X;
		mLastMove->to_Y = move->to_Y;

		//Record the move.
		MoveManager::Instance()->StoreMove(*move);

		//Piece is in a new position.
		mSelectedPiecePosition->x = move->to_X;
		mSelectedPiecePosition->y = move->to_Y;
	}

	//Check if we need to promote a pawn.
	if (board->currentLayout[move->to_X][move->to_Y].piece == PIECE_PAWN && (move->to_Y == 0 || move->to_Y == 7))
	{
		//Time to promote - Always QUEEN at the moment.
		PIECE newPieceType = PIECE_QUEEN;

		//Change the PAWN into the selected piece.
		board->currentLayout[move->to_X][move->to_Y].piece = newPieceType;
	}

	//Not finished turn yet.
	return true;
}

//--------------------------------------------------------------------------------------------------

bool ChessPlayerAI::TakeATurn(SDL_Event e)
{
	//TODO: Code your own function - Remove this version after, it is only here to keep the game functioning for testing.

	DWORD startTime		 = GetTickCount();
	DWORD currentTime	 = 0;

	Move move;
	Node node;

	node.boardState = *mChessBoard;
	node.currentMove = *mLastMove;
	node.score = 0;

	Node bestChildNode = MiniMax(node, 0);

	bool gameStillActive = MakeAMove(mChessBoard, &bestChildNode.currentMove);
	currentTime = GetTickCount();
	cout << " - [AI Time taken: " << std::setprecision(10) << (currentTime-startTime)/1000.0f << " seconds]";
	return gameStillActive;

	//-----------------------------------------------------------
}

//--------------------------------------------------------------------------------------------------

Node ChessPlayerAI::MiniMax(Node node, int depth)
{
	return Maximise(node, depth, 0, 0);
}

//--------------------------------------------------------------------------------------------------

Node ChessPlayerAI::Maximise(Node node, int depth, int a, int b)
{
	Node bestChildNode;
	Node bestRealNode;
	Node tmpNode;
	vector<Move> moves;
	int alpha = a;
	int beta = b;

	//If we are at the bottom of the tree
	if (depth >= *mDepthToSearch) {
		return node;
	}

	//Otherwise, we generate all the possible moves from the current state
	GetAllMoveOptions(node.boardState, mTeamColour, &moves);

	int i = 0;

	//For each state, we create a new node (board + currentMove) and call minimise
	for (auto move : moves)
	{
		Node childNode;
		childNode.score = 0;
		childNode.currentMove = move;
		childNode.boardState = node.boardState;

		MakeAMove(&childNode.boardState, &move);
		childNode.score = ScoreTheBoard(childNode);

		if (i == 0)
		{
			bestRealNode = childNode;
		}

		tmpNode = Minimise(childNode, depth + 1, alpha, beta);

		if (tmpNode.score > bestChildNode.score)
		{
			bestChildNode = tmpNode;
			bestRealNode = childNode;
			alpha = EvaluateBoard(node, COLOUR_BLACK);
		}

		if (beta > alpha)
		{
			return bestRealNode;
		}
	}

	return bestRealNode;
}

//--------------------------------------------------------------------------------------------------

Node ChessPlayerAI::Minimise(Node node, int depth, int a, int b)
{
	Node bestChildNode;
	Node bestRealNode;
	Node tmpNode;
	vector<Move> moves;
	int alpha = a;
	int beta = b;

	//If we are at the bottom of the tree
	if (depth >= *mDepthToSearch) {
		return node;
	}

	//Otherwise, we generate all the possible moves from the current state
	GetAllMoveOptions(node.boardState, mTeamColour, &moves);

	int i = 0;

	//For each state, we create a new node (board + currentMove) and call minimise
	for (auto move : moves)
	{
		Node childNode;
		childNode.score = 0;
		childNode.currentMove = move;
		childNode.boardState = node.boardState;
		
		MakeAMove(&childNode.boardState, &move);
		childNode.score = ScoreTheBoard(childNode);

		if (i == 0)
		{
			bestRealNode = childNode;
		}

		tmpNode = Maximise(childNode, depth + 1, alpha, beta);

		if (tmpNode.score < bestChildNode.score)
		{
			bestChildNode = tmpNode;
			bestRealNode = childNode;
			beta = EvaluateBoard(node, COLOUR_WHITE);
		}

		if (beta < alpha)
		{
			return bestRealNode;
		}
	}

	return bestRealNode;
}

//--------------------------------------------------------------------------------------------------

void ChessPlayerAI::OrderMoves(Board* board, vector<Move>* moves, bool highToLow)
{
	//TODO
}

//--------------------------------------------------------------------------------------------------

void ChessPlayerAI::CropMoves(vector<Move>* moves, unsigned int maxNumberOfMoves)
{
	//TODO
}

//--------------------------------------------------------------------------------------------------

int ChessPlayerAI::EvaluateBoard(Node node, COLOUR colour)
{
	int score = 0;

	for (int j = 0; j < kBoardDimensions; j++)
	{
		for (int i = 0; i < kBoardDimensions; i++)
		{
			if (node.boardState.currentLayout[i][j].colour == colour)
			{
				score += getScoreByPieceType(&node.boardState.currentLayout[i][j]) * kPieceWeight;
				score += getScoreByPiecePosition(i, j, &node.boardState.currentLayout[i][j]) * kPositionalWeight;
			}
		}
	}

	score += getScoreByPieceMoves(&node.boardState, colour);
	score += getScoreByPieceAttacks(&node.boardState, colour);

	return score;
}

int ChessPlayerAI::ScoreTheBoard(Node node)
{
	int robertScore = 0;
	int scoreBlack = EvaluateBoard(node, COLOUR_BLACK);
	int scoreWhite = EvaluateBoard(node, COLOUR_WHITE);

	//We score the black

	if (mTeamColour == COLOUR_BLACK)
	{
		robertScore = scoreBlack - scoreWhite;
	}
	else
	{
		robertScore = scoreWhite - scoreBlack;
	}

	return robertScore;
}

//--------------------------------------------------------------------------------------------------

int ChessPlayerAI::getScoreByPieceType(BoardPiece* piece)
{
	switch (piece->piece)
	{
	case PIECE_PAWN:
		return kPawnScore;
		break;

	case PIECE_KNIGHT:
		return kKnightScore;
		break;

	case PIECE_BISHOP:
		return kBishopScore;
		break;

	case PIECE_ROOK:
		return kRookScore;
		break;

	case PIECE_QUEEN:
		return kQueenScore;
		break;

	case PIECE_KING:
		return kKingScore;
		break;
	}

	return 0;
}

//--------------------------------------------------------------------------------------------------

int ChessPlayerAI::getScoreByPiecePosition(int x, int y, BoardPiece* piece)
{
	int score = 0;

	if (x >= 3 && x <= 5)
	{
		if (y >= 3 && x <= 5)
		{
			score = 5;
		}
		else if (x >= 1 && x <= 6)
		{
			score = 3;
		}
	}
	else if(x >= 1 && x <= 6)
	{
		if (y >= 3 && x <= 5)
		{
			score = 3;
		}
		else if (x >= 1 && x <= 6)
		{
			score = 1;
		}
	}

	return 0;
}

//--------------------------------------------------------------------------------------------------

int ChessPlayerAI::getScoreByPieceMoves(Board* board, COLOUR colour)
{
	int score = 0;
	vector<Move> moves;
	
	GetAllMoveOptions(*board, colour, &moves);

	for(auto move : moves)
	{
		score++;
	}

	return score;
}

//--------------------------------------------------------------------------------------------------

int ChessPlayerAI::getScoreByPieceAttacks(Board* board, COLOUR colour)
{
	int score = 0;
	vector<Move> moves;

	GetAllMoveOptions(*board, colour, &moves);

	for (auto move : moves)
	{
		if (board->currentLayout[move.to_X][move.to_Y].piece != PIECE_NONE)
		{
			score += getScoreByPieceType(&board->currentLayout[move.to_X][move.to_Y]);
		}
	}

	return score;
}

//--------------------------------------------------------------------------------------------------