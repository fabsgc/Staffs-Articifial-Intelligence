#include "Layer.h"

Layer::Layer(UINT numberNeurons, UINT numberInputsNeuron)
	: _numberNeurons(numberNeurons)
	, _numberInputsNeuron(numberInputsNeuron)
{
}

Layer::~Layer()
{
}

void Layer::Initialise()
{
	for (UINT i = 0; i < _numberNeurons; i++)
	{
		NeuronPtr neuron(new Neuron(_numberInputsNeuron));
		neuron->Initialise();
		_neurons.push_back(neuron);
	}
}

void Layer::SetInputs(std::vector<float>& inputs)
{
	_inputs = inputs;
}

std::vector<float>& Layer::GetInputs()
{
	return _inputs;
}

std::vector<float>& Layer::GetOutputs()
{
	return _outputs;
}

std::vector<NeuronPtr>& Layer::GetNeurons()
{
	return _neurons;
}