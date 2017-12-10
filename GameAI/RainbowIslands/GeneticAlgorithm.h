#pragma once

#include "NeuralNetworkHeader.h"
#include "Genome.h"

class GeneticAlgorithm
{
public:
	GeneticAlgorithm();
	~GeneticAlgorithm();

	void Reset();

	void CalculateFitness();

	void Selection();
	void Crossover();
	void Mutation();

private:
	std::vector<Genome> _genomes;
	std::vector<Genome> _selectedGenomes;
};