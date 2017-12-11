#pragma once

#include "NeuralNetworkHeader.h"

class Genome
{
public:
	Genome();
	~Genome();

	std::vector<float>& GetWeights();
	float               GetFitness() const;

	void                SetWeights(std::vector<float>& weights);
	void                SetFitness(float fitness);

public:
	friend bool operator<(const Genome& lhs, const Genome& rhs)
	{
		return (lhs.GetFitness() < rhs.GetFitness());
	}

private:
	std::vector<float> _weights;
	float              _fitness;
};

typedef std::shared_ptr<Genome> GenomePtr;