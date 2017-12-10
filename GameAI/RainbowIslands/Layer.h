#ifndef _LAYER_H
#define _LAYER_H

#include "NeuralNetworkHeader.h"
#include "Neuron.h"

class Layer
{
public:
	Layer(UINT numberNeurons, UINT numberInputsNeuron);
	~Layer();

	void                    Initialise();
	void                    SetInputs(std::vector<float>& inputs);

	std::vector<float>&     GetInputs();
	std::vector<float>&     GetOutputs();

	std::vector<NeuronPtr>& GetNeurons();

private:
	UINT                   _numberNeurons;
	UINT                   _numberInputsNeuron;

	std::vector<NeuronPtr> _neurons;

	std::vector<float>     _inputs;
	std::vector<float>     _outputs;
};

typedef std::shared_ptr<Layer> LayerPtr;

#endif