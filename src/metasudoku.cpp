#include "common.h"
namespace meta {
	class Genome
	{
	public:
		int score;
		int params[4];

		Genome()
		{
			score = -1;
		}

		void single_crossover(mGenome &g)
		{
			int x = randrange(1, 3) * 4;
			fr (i, x) swap(params[i], g.params[i]);
			score= g.score = -1;
		}

		void mutate()
		{
			int from;
			ini(con_mask, 0);

		}
	};
}
