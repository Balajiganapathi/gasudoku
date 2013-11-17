#include "common.h"
#include <random>
#include <pthread.h>

class mGenome;

extern int gen, gen_limit, restarts, display_level;

int mgen;
int mmin_gen;
int mpop_len = 50;
int mpop_retain = mpop_len * 0.25;
int mrestarts = 0;
int mrestart_limit = 10;
int mgen_limit = 10000;
double mmutation_rate         = 0.9;
double msingle_crossover_rate = 0.9;
double mfitter_parent = 0.25;
mGenome *mpop;
mt19937_64 mrand;
char cases[100][100];
double weights[4] = {1,0.25,0.5,1};
FILE* flog;

int fork_handler(int params[], int i, int seed = 0)
{
	fclose(flog);
	fclose(stderr);
	initMetaParams(params, i, seed);

	return (int(weights[i]*(restarts*gen_limit + gen)));
}

int mgetRandom(int x)
{
	switch(x)
	{
		case 0:
			return mrand() % 25 + 75;
		case 1:
		case 2:
		case 3:
			return mrand() % 900 + 100;
		default:
			return -1;
	}
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
		int x = randrange(0, 3);
		fr (i, x) swap(params[i], g.params[i]);
		score = g.score = -1;
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
		score = 0;
		int children[2][4];
		for (int i = 0; i < 4; i++)
		{
			if ((children[0][i] = fork()) == 0)
				exit(fork_handler(params, i, mrand()));
		}
		for (int i = 0; i < 4; i++)
		{
			waitpid(children[0][i], &children[1][i], 0);
			score += children[1][i];
		}
		score /= 4;
		cout << "gen = " << score << endl;
		return score;
	}
	bool operator <(const mGenome &g) const
	{
	    assert(score != -1 && g.score != -1);
	    return score < g.score;
    }
};

void * thread_handler(void * genome)
{
	mGenome *g = (mGenome *) genome;
	g->get_score();
	return NULL;
}

int mselect()
{
	int a = randrange(0, mpop_len - 1), b;
	b     = a;
	while (b == a)
		b = randrange(0, mpop_len - 1);
	if (mpop[a].get_score() > mpop[b].get_score()) 
		swap(a, b);
	if(fire(mfitter_parent)) 
		return a;
	else 
		return b;
}

void marrange_min()
{
	for (int i = 0; i < mpop_retain; ++i)
		if (mpop[0].get_score() > mpop[i].get_score())
			swap(mpop[0], mpop[i]);
}

void mwrapGA()
{
    delete[] mpop;
}

void mdisp_progress() 
{

	fprintf(flog, "%d %d", mgen, mpop[0].get_score());
	for (int i = 0; i < 4; i++)
		fprintf(flog, " %d", mpop[0].params[i]);
	fprintf(flog, "\n");
	if(display_level <= 0) return;
    if(display_level <= 1) {
        if(mgen == 0) printf("mRESTART: %d\n", mrestarts);
        return;
    }

    printf("I am at gen % 4d with best score % 4d: ", mgen, mpop[0].get_score());
    fr(i, 4) printf("%d ", mpop[0].params[i]);
    printf("\n");
}

void minitGA()
{
	int i;
	pthread_t tid, tids[mpop_len];
	for (i = 0; i < mpop_len; ++i) 
	{
		mpop[i].fillrandom();
		tid = pthread_create(tids + i, NULL, thread_handler, &mpop[i]);
		while (tid != 0)
		{
			tid = pthread_create(tids + i, NULL, thread_handler, &mpop[i]);
		}
	}
	for (i = 0; i < mpop_len; i++)
	{

		pthread_join(tids[i], NULL);
		if (mpop[i].get_score() < mmin_gen)
		{
			swap(mpop[i], mpop[0]);
			return;
		}
	}

	nth_element(mpop, mpop + mpop_retain, mpop + mpop_len);
    marrange_min();
	mdisp_progress();
	mgen = 0;
}

void mnextgen()
{
	++mgen;
	for(int i = mpop_retain; i < mpop_len; i += 2) 
	{
		int a = mselect(), b = mselect();
		while (a == b) b = mselect();
		mpop[i] = mpop[a]; 
		mpop[i + 1] = mpop[b];
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

mGenome msolve()
{
	int prev_5[5] = {0}, i = 0;
	mGenome res;
	mpop = new mGenome[mpop_len];
	while (mrestarts < mrestart_limit) 
	{
		mgen = -1;
		mrand.seed(int(clock()));
		minitGA();
        while((mpop[0].get_score() > mmin_gen) && (mgen < mgen_limit))
		{
            mnextgen();
        }
        res = mpop[0];
        if(res.get_score() <= mmin_gen) 
			break;
	    ++mrestarts;
	}
    mwrapGA();
	return res;
}

int metaGA(int argc, char* argv[])
{
	if (argc > 2)
		mmin_gen = strtol(argv[2], NULL, 10);
	else
		mmin_gen = 500;
	int i       = 0;
	FILE *input = fopen("all_cases.txt", "r");
	while(fscanf(input, "%s", cases[i++]) != EOF)
		;
	fclose(input);
	initParams();
	flog  = fopen("meta.log","w");
	cout << "Starting metaGA." << endl;
	
	mrand.seed(strtol(argv[1], NULL, 10));
	mGenome ans = msolve();
	cerr << "gen = " << ans.get_score() << '\n';
	cerr << "fitter_parent = " << ans.params[0] << '\n';
	cerr << "pop_retain = " << ans.params[1] << '\n';
	cerr << "mutation_rate = " << ans.params[2] << '\n';
	cerr << "single_crossover_rate = " << ans.params[3] << endl;
	fclose(flog);
	return 0;
}
