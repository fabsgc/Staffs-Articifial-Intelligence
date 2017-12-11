#include "Genome.h"

Genome::Genome()
	: _fitness(0.0f)
{
}

Genome::~Genome()
{
}

std::vector<float>& Genome::GetWeights()
{
	return _weights;
}

float Genome::GetFitness() const
{
	return _fitness;
}

void Genome::SetWeights(std::vector<float>& weights)
{
	_weights = weights;
}

void Genome::SetFitness(float fitness)
{
	_fitness = fitness;
}