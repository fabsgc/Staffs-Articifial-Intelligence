#pragma once

#include "NeuralNetworkHeader.h"

class Genome
{
public:
	Genome();
	~Genome();

	std::vector<float>& GetWeights();
	std::vector<float>& GetInputs();
	std::vector<float>& GetOutputs();
	float&              GetReward();
	float               GetFitness() const;

	void                SetWeights(std::vector<float>& weights);
	void                SetInputs(std::vector<float>& inputs);
	void                SetOutputs(std::vector<float>& outputs);
	void                SetReward(float reward);
	void                SetFitness(float fitness);

public:
	friend bool operator<(const Genome& lhs, const Genome& rhs)
	{
		return (lhs.GetFitness() < rhs.GetFitness());
	}

private:
	std::vector<float> _weights;
	std::vector<float> _inputs;
	std::vector<float> _outputs;
	float              _reward;
	float              _fitness;
};