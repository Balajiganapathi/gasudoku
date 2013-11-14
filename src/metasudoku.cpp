#include "common.h"
namespace meta {
	double mmutation_rate, msingle_crossover_rate, mfitter_parent;
	class mGenome
	{
	public:
		int score;
		int params[4];

		mGenome()
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
			for (int i = 0; i < 81; i++)
			{
				if (col(i) == 0)
				{
	                from = oo;
					if(fire(mutation_rate)) 
					{
			            from = i + randrange(0, 3);
		                score = -1;
	                }
				}
				if (i >= from) params[i] = getRandom(i);
			}
		}

		void fillrandom() 
		{
			ini(con_mask, 0);
			for (int i = 0; i < 81; ++i)
			{
				params[i] = getRandom(i);
				assert(params[i] >= 0);
			}
			score = -1;
		}

		int get_score()
		{
			if (score != -1) return score;
			initParams();
			initMetaParams(params);
			processInput();
		    Genome ans = solve();
			score = gen;
			return score;
		}
		bool operator <(const mGenome &g) const {
		    assert(score != -1 && g.score != -1);
		    return score < g.score;
	    }
	};
	int mselect()
	{
		int a = randrange(0, pop_len - 1), b;
		b     = a;
		while (b == a)
			b = randrange(0, pop_len - 1);
		if (mpop[a].get_score() > mpop[b].get_score()) 
			swap(a, b);
		if(fire(fitter_parent)) 
			return a;
		else 
			return b;
	}

	void marrange_min()
	{
		for (int i = 0; i < pop_len; ++i)
			if (pop[0].get_score() > pop[i].get_score())
				swap(mpop[0], mpop[i]);
	}

	void minitGA()
	{
		mpop = new mGenome[pop_len];
		for (int i = 0; i < pop_len; ++i) 
		{
			mpop[i].fillrandom();
			if (mpop[i].get_score() == 0)
			{
				swap(mpop[i], mpop[0]);
				return;
			}
		}

		mgen = 0;
		mnth_element(mpop, mpop + mpop_retain, mpop + mpop_len);
	    marrange_min();
		mdisp_progress();
	}
	
	void mwrapGA()
	{
	    delete[] mpop;
	}

}

void initMetaParams(int params[])
{
	srand(params[0]);
    pop_retain = param[1];
    mutation_rate = param[2]/1000.0;
    single_crossover_rate = params[3]/1000.0;
}
