#include "common.h"

int mgen, mpop_retain = 190, mpop_len = 200;
double mmutation_rate, msingle_crossover_rate, mfitter_parent;
mGenome *mpop;

void initMetaParams(int params[])
{
	srand(params[0]);
    pop_retain = params[1];
    mutation_rate = params[2]/1000.0;
    single_crossover_rate = params[3]/1000.0;
}

int mgetRandom(int x)
{
	int c;
	return c;
}

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
		for (int i = 0; i < 4; i++)
		{
			if (params[i] == 0)
			{
                from = oo;
				if(fire(mmutation_rate)) 
				{
		            from = i + randrange(0, 3);
	                score = -1;
                }
			}
			if (i >= from) params[i] = mgetRandom(i);
		}
	}

	void fillrandom() 
	{
		for (int i = 0; i < 4; ++i)
		{
			params[i] = mgetRandom(i);
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
	    solve();
		score = gen;
		return score;
	}
	bool operator <(const mGenome &g) const
	{
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
		if (mpop[0].get_score() > mpop[i].get_score())
			swap(mpop[0], mpop[i]);
}

void mwrapGA()
{
    delete[] mpop;
}

void mdisp_progress() 
{
    printf("I am at gen % 4d with best score % 4d: ", mgen, mpop[0].get_score());
    fr(i, 4) printf("%d", mpop[0].params[i]);
    printf("\n");
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
	nth_element(mpop, mpop + pop_retain, mpop + pop_len);
    marrange_min();
	mdisp_progress();
}

void mnextgen()
{
	++mgen;
	for(int i = mpop_retain; i < mpop_len; i += 2) 
	{
		int a = mselect(), b = mselect();
		while(a == b) b = mselect();
		mpop[i] = mpop[a]; mpop[i + 1] = mpop[b];
		if (fire(msingle_crossover_rate)) 
		{
			mpop[i].single_crossover(mpop[i + 1]);
			mpop[i].mutate();
			mpop[i + 1].mutate();
		} 
		else 
		{
			mpop[i].fillrandom();
			mpop[i + 1].fillrandom();
		}

		mpop[i].get_score();
		mpop[i + 1].get_score();
    }

    nth_element(mpop, mpop + mpop_retain, mpop + mpop_len);
	marrange_min();
	mdisp_progress();
}

int metaGA(int argc, char* argv[])
{
	return 0;
}
