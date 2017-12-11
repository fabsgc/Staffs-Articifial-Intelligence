#pragma once

#include "NeuralNetworkHeader.h"
#include "Constants_RainbowIslands.h"
#include "Genome.h"

class GeneticAlgorithm
{
public:
	GeneticAlgorithm();
	~GeneticAlgorithm();

	void AddGenome(GenomePtr& genome);

	std::vector<GenomePtr>& GetGenomes() { return _genomes;  }

	void Reset();

	void CalculateFitness();

	void Selection();
	void Crossover();
	void Mutation();

private:
	std::vector<GenomePtr> _genomes;
	std::vector<GenomePtr> _selectedGenomes;
};

typedef std::shared_ptr<GeneticAlgorithm> GeneticAlgorithmPtr;