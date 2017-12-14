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
	UINT                GetNumberWeights();
	UINT                GetNumberInputs();
	float&              GetOutput();

	void                SetWeights(std::vector<float>& weights);
	void                SetOutput(float output);

public:
	static const float DefaultBias;
	static const float DefaultActivation;

private:
	std::vector<float> _weights;
	UINT               _numberInputs;

	float              _ativation;
	float              _output;
};

typedef std::shared_ptr<Neuron> NeuronPtr;

#endif