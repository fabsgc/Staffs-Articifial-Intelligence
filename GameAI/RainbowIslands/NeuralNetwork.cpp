#include "NeuralNetwork.h"

NeuralNetwork::NeuralNetwork(UINT numberInputNeurons, UINT numberHiddenNeurons, UINT numberOutputNeurons)
	: _numberInputNeurons(numberInputNeurons)
	, _numberHiddenNeurons(numberHiddenNeurons)
	, _numberOutputNeurons(numberOutputNeurons)
	, _neuralNetwork(this)
	, _learningRate(0.5)
{
}

NeuralNetwork::~NeuralNetwork()
{
}

void NeuralNetwork::Initialise()
{
	AddLayer(LayerPtr(new Layer(_numberInputNeurons, _numberHiddenNeurons)));
	AddLayer(LayerPtr(new Layer(_numberOutputNeurons, _numberHiddenNeurons)));

	for (auto layer : _layers)
	{
		layer->Initialise();
	}
}

void NeuralNetwork::Run()
{
	//For each layer
	for (UINT i = 0; i < _layers.size(); i++)
	{
		std::vector<NeuronPtr>& neurons = _layers[i]->GetNeurons();
		std::vector<float> outputs;

		if (i > 0)
		{
			_inputs = _outputs;
		}

		//For each neuron
		for (UINT j = 0; j < neurons.size(); j++)
		{
			std::vector<float> weights = neurons[j]->GetWeights();
			UINT numberInputs          = neurons[0]->GetNumberInputs();
			float neuronInput          = 0.0f;

			for (UINT k = 0; k < numberInputs - 1; ++k)
			{
				neuronInput += weights[0] * _inputs[k];
			}

			neuronInput += weights[numberInputs - 1] * Neuron::DefaultBias;
			outputs.push_back(Sigmoid(neuronInput));
		}

		_outputs = outputs;
	}
}

void NeuralNetwork::SetInputs(std::vector<float>& inputs)
{
	_inputs = inputs;
}

void NeuralNetwork::SetTargets(std::vector<float>& targets)
{
	_targets = targets;
}

std::vector<float>& NeuralNetwork::GetOutputs()
{
	return _outputs;
}

std::vector<float>& NeuralNetwork::GetTargets()
{
	return _targets;
}

void NeuralNetwork::AddLayer(LayerPtr layer)
{
	_layers.push_back(layer);
}

float NeuralNetwork::Sigmoid(float activation, float response)
{
	return (1.0f / (1.0f + pow(2.7183f, activation / response)));
}