#include "Genome.h"

Genome::Genome()
	: _fitness(0.0f)
	, _reward(0.0f)
{
}

Genome::~Genome()
{
}

std::vector<float>& Genome::GetWeights()
{
	return _weights;
}

std::vector<float>& Genome::GetInputs()
{
	return _inputs;
}

std::vector<float>& Genome::GetOutputs()
{
	return _outputs;
}

float& Genome::GetReward()
{
	return _reward;
}

float Genome::GetFitness() const
{
	return _fitness;
}

void Genome::SetWeights(std::vector<float>& weights)
{
	_weights = weights;
}

void Genome::SetInputs(std::vector<float>& inputs)
{
	_inputs = inputs;
}

void Genome::SetOutputs(std::vector<float>& outputs)
{
	_outputs = outputs;
}

void Genome::SetReward(float reward)
{
	_reward = reward;
}

void Genome::SetFitness(float fitness)
{
	_fitness = fitness;
}