#include "Neuron.h"

const float Neuron::DefaultBias       = 0.5f;
const float Neuron::DefaultActivation = 0.5f;

Neuron::Neuron(UINT numberInputs)
	: _numberInputs(numberInputs + 1)
{
}

Neuron::~Neuron()
{
}

void Neuron::Initialise()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1, 1);

	for (UINT i = 0; i < _numberInputs; i++)
	{
		_weights.push_back((float)dis(gen));
	}
}

std::vector<float>& Neuron::GetWeights()
{
	return _weights;
}

UINT Neuron::GetNumberInputs()
{
	return _numberInputs;
}

UINT Neuron::GetNumberWeights()
{
	return _weights.size();
}

void Neuron::SetWeights(std::vector<float>& weights)
{
	_weights = weights;
}