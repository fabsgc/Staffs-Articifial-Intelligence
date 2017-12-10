#ifndef _NEURON_H
#define _NEURON_H

#include "NeuralNetworkHeader.h"

class Neuron
{
public:
	Neuron(UINT numberInputs);
	~Neuron();

	void                Initialise();
	std::vector<float>& GetWeights();
	UINT                GetNumberInputs();

public:
	static const float DefaultBias;
	static const float DefaultActivation;

private:
	std::vector<float> _weights;
	UINT               _numberInputs;
};

typedef std::shared_ptr<Neuron> NeuronPtr;

#endif