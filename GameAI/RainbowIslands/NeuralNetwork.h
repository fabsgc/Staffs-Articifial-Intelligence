#ifndef _NEURALNETWORK_H
#define _NEURALNETWORK_H

#include "NeuralNetworkHeader.h"
#include "Layer.h"

class NeuralNetwork
{
public:
	NeuralNetwork(UINT numberInputNeurons, UINT numberHiddenNeurons, UINT numberOutputNeurons);
	~NeuralNetwork();

	void                Initialise();
	void                Run();

	void                SetInputs(std::vector<float>& inputs);
	void                SetTargets(std::vector<float>& targets);
	void                SetWeights(std::vector<float>& weights);

	std::vector<float>& GetTargets();
	std::vector<float>& GetOutputs();
	std::vector<float>  GetWeights();

	float               Sigmoid(float activation, float response = 1.0f);

private:
	void                AddLayer(LayerPtr layer);

private:
	std::vector<float>    _inputs;
	std::vector<float>    _outputs;
	std::vector<float>    _targets;

	std::vector<LayerPtr> _layers;
	
	UINT                  _numberInputNeurons;
	UINT                  _numberHiddenNeurons;
	UINT                  _numberOutputNeurons;

	NeuralNetwork*        _neuralNetwork;

	float                 _learningRate;
};

typedef std::shared_ptr<NeuralNetwork> NeuralNetworkPtr;

#endif