//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#ifndef _CHESSPLAYERAI_H
#define _CHESSPLAYERAI_H

#include "ChessPlayer.h"
#include "ChessCommons.h"
#include "../Commons.h"
#include <SDL.h>
#include <assert.h>

class ChessPlayerAI : public ChessPlayer
{
//--------------------------------------------------------------------------------------------------
public:
	ChessPlayerAI(SDL_Renderer* renderer, COLOUR colour, Board* board, vector<Vector2D>* highlights, Vector2D* selectedPiecePosition, Move* lastMove, int* searchDepth);
	~ChessPlayerAI();

	bool		TakeATurn(SDL_Event e);

//--------------------------------------------------------------------------------------------------
protected:
	Node MiniMax(Node node, int depth);
	Node Maximise(Node node, int depth, int a, int b);
	Node Minimise(Node node, int depth, int a, int b);
	bool MakeAMove(Board* board, Move* move);

	void OrderMoves(Board* board, vector<Move>* moves, bool highToLow);
	void CropMoves(vector<Move>* moves, unsigned int maxNumberOfMoves);

	int  ScoreTheBoard(Node node);
	int EvaluateBoard(Node node, COLOUR colour);

	int getScoreByPieceType(BoardPiece* piece);
	int getScoreByPiecePosition(int x, int y, BoardPiece* piece); //TODO
	int getScoreByPieceMoves(Board* board, COLOUR colour);
	int getScoreByPieceAttacks(Board* board, COLOUR colour);

private:
	int* mDepthToSearch;
};


#endif //_CHESSPLAYERAI_H