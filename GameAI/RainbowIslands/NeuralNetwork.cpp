#include "NeuralNetwork.h"

NeuralNetwork::NeuralNetwork(UINT numberInputNeurons, UINT numberHiddenNeurons, UINT numberOutputNeurons)
	: _numberInputNeurons(numberInputNeurons)
	, _numberHiddenNeurons(numberHiddenNeurons)
	, _numberOutputNeurons(numberOutputNeurons)
	, _neuralNetwork(this)
	, _learningRate(0.7)
{
}

NeuralNetwork::~NeuralNetwork()
{
}

void NeuralNetwork::Initialise()
{
	AddLayer(LayerPtr(new Layer(_numberHiddenNeurons, _numberInputNeurons)));
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

			for (UINT k = 0; k < numberInputs - 1; k++)
			{
				neuronInput += weights[k] * _inputs[k];
			}

			neuronInput += weights[numberInputs - 1] * Neuron::DefaultBias;
			float output = Sigmoid(neuronInput);
			outputs.push_back(output);
			neurons[j]->SetOutput(output);
		}

		_outputs = outputs;
	}
}

void NeuralNetwork::BackPropagation(const std::vector<float>& targets, const std::vector<float>& inputs)
{
	//Output layer
	std::vector<NeuronPtr>& hiddenNeurons = _layers[0]->GetNeurons();
	std::vector<NeuronPtr>& outputNeurons = _layers[1]->GetNeurons();
	std::vector<float> outputErrors;

	//For each neuron in output layer
	for (UINT i = 0; i < outputNeurons.size(); i++)
	{
		float output = outputNeurons[i]->GetOutput();
		//Ek = (tk - ok) * ok (1 - ok)
		float error = output - targets[i]; //* output - target
		outputErrors.push_back(error);

		std::vector<float> weights = outputNeurons[i]->GetWeights();
		
		for (UINT j = 0; j < outputNeurons.size(); j++)
		{
			//Wjk = L * Ek * oj
			weights[j] += _learningRate * error * hiddenNeurons[j]->GetOutput();
		}
		//Wjk = L * Ek * oj (bias)
		weights[outputNeurons.size()] += _learningRate * error * Neuron::DefaultBias;
		outputNeurons[i]->SetWeights(weights);
	}

	//For each neuron in hidden layer
	for (UINT i = 0; i < hiddenNeurons.size(); i++)
	{
		//Ej = ok(1 - ok) * sum(Ek * Wjk)
		float hiddenError = 0.0f;
		for (UINT j = 0; j < outputErrors.size(); j++)
		{
			hiddenError += outputErrors[j] * outputNeurons[j]->GetWeights()[i];
		}
		float output = hiddenNeurons[i]->GetOutput();
		hiddenError *= (1.0f - output);

		std::vector<float> hiddenWeights = hiddenNeurons[i]->GetWeights();
		for (UINT j = 0; j < inputs.size(); j++)
		{
			hiddenWeights[j] += _learningRate * hiddenError * inputs[j];
		}
		hiddenWeights[inputs.size()] += _learningRate * hiddenError * Neuron::DefaultBias;
	}

	//_learningRate = _learningRate * 0.9;

	//Hidden layer
}

void NeuralNetwork::SetInputs(const std::vector<float>& inputs)
{
	_inputs = inputs;
}

void NeuralNetwork::SetTargets(const std::vector<float>& targets)
{
	_targets = targets;
}

void NeuralNetwork::SetWeights(const std::vector<float>& weights)
{
	int currentWeight = 0;
	auto weightBeginIterator = weights.begin();
	auto weightEndIterator   = weights.begin();

	for (auto layer : _layers)
	{
		std::vector<NeuronPtr> neurons = layer->GetNeurons();

		for (auto neuron : neurons)
		{
			UINT numberWeight = neuron->GetNumberWeights();
			weightEndIterator += numberWeight;

			std::vector<float> currentWeights(weightBeginIterator, weightEndIterator);

			weightBeginIterator = weightEndIterator;
			weightEndIterator = weightBeginIterator;
		}
	}
}

void NeuralNetwork::SetLearningRate(const float& learningRate)
{
	_learningRate = learningRate;
}

std::vector<float>& NeuralNetwork::GetOutputs()
{
	return _outputs;
}

std::vector<float>& NeuralNetwork::GetTargets()
{
	return _targets;
}

std::vector<float> NeuralNetwork::GetWeights()
{
	std::vector<float> weights;

	for (auto layer : _layers)
	{
		std::vector<NeuronPtr> neurons = layer->GetNeurons();

		for (auto neuron : neurons)
		{
			std::vector<float> weightsTmp = neuron->GetWeights();
			weights.insert(weights.end(), weightsTmp.begin(), weightsTmp.end());
		}
	}

	return weights;
}

float& NeuralNetwork::GetLearningRate()
{
	return _learningRate;
}

void NeuralNetwork::AddLayer(LayerPtr layer)
{
	_layers.push_back(layer);
}

float NeuralNetwork::Sigmoid(float activation, float response)
{
	return (1.0f / (1.0f + pow(2.7183f, activation / response)));
}