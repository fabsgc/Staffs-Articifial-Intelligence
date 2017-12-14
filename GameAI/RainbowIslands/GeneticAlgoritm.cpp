#include "GeneticAlgorithm.h"

GeneticAlgorithm::GeneticAlgorithm()
{
}

GeneticAlgorithm::~GeneticAlgorithm()
{
}

void GeneticAlgorithm::AddGenome(GenomePtr& genome)
{
	_genomes.push_back(genome);
}

void GeneticAlgorithm::Reset()
{
	_genomes.clear();
	_selectedGenomes.clear();
}

void GeneticAlgorithm::CalculateFitness()
{
	float maxFitness = 0.0f;

	//We get max fitness value
	for (auto genome : _genomes)
		if (genome->GetFitness() > maxFitness)
			maxFitness = genome->GetFitness();

	//We normalise fitness
	for (auto genome : _genomes)
		genome->SetFitness(genome->GetFitness() / maxFitness);

	Selection();
}

void GeneticAlgorithm::Selection()
{
	//We get genome which have a quite good fitness
	for (auto genome : _genomes)
	{
		if (genome->GetFitness() > 0.7f)
		{
			_selectedGenomes.push_back(genome);
		}
	}

	if (_selectedGenomes.size() < 2)
	{
		_selectedGenomes.push_back(_genomes[0]);
		_selectedGenomes.push_back(_genomes[1]);
	}

	Crossover();
}

void GeneticAlgorithm::Crossover()
{
	//We use the easiest crossover method for the moment
	GenomePtr parent[2];
	int numberSelected = _selectedGenomes.size();
	int numberElements = _selectedGenomes[0]->GetWeights().size();

	for (int i = numberSelected; i < kPopulationSize; i++)
	{
		GenomePtr genome(new Genome());

		parent[0] = _selectedGenomes.at((rand() % numberSelected));
		parent[1] = _selectedGenomes.at((rand() % numberSelected));

		if (rand() % (10000 / kNNCrossoverRate))
		{
			auto parent0Start = parent[0]->GetWeights().begin();
			auto parent0End   = parent[0]->GetWeights().begin() + (numberElements / 2);

			auto parent1Start = parent[0]->GetWeights().begin() + (numberElements / 2);
			auto parent1End = parent[0]->GetWeights().end();

			std::vector<float> weights;

			weights.insert(weights.end(), parent0Start, parent0End);
			weights.insert(weights.end(), parent1Start, parent1End);

			GenomePtr genome(new Genome());
			genome->SetWeights(weights);
			_selectedGenomes.push_back(genome);
		}
		else
		{
			std::vector<float> weights = _genomes.at((rand() % _genomes.size()))->GetWeights();
			genome->SetWeights(weights);
			_selectedGenomes.push_back(genome);
		}
	}

	Mutation();
}

void GeneticAlgorithm::Mutation()
{
	for (auto genome : _selectedGenomes)
	{
		std::vector<float> weights = genome->GetWeights();

		for(auto i = weights.begin(); i < weights.end(); i++)
		{
			if (rand() % 10000 < kNNMutationRate)
			{
				*i = rand() % 2 - 1.0f;
			}
		}
	}

	_genomes = _selectedGenomes;
}