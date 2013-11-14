//Coder: Balajiganapathi
#define TRACE
#define DEBUG

#include <algorithm>
#include <bitset>
#include <deque>
#include <cassert>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <utility>
#include <vector>
#include <random>

using namespace std;

typedef long long ll;
typedef vector<int> vi;
typedef pair<int,int> pi;
typedef vector<string> vs;

// Basic macros
#define st          first
#define se          second
#define all(x)      (x).begin(), (x).end()
#define ini(a, v)   memset(a, v, sizeof(a))
#define re(i,s,n)  	for(int i=s;i<(n);++i)
#define rep(i,s,n)  for(int i=s;i<=(n);++i)
#define fr(i,n)     re(i,0,n)
#define tr(i,x)     for(typeof(x.begin()) i=x.begin();i!=x.end();++i)
#define pu          push_back
#define mp          make_pair
#define sz(x)       (int)(x.size())

const int oo = 2000000009;
const double eps = 1e-9;

#ifdef TRACE
    #define trace1(x)                cerr << #x << ": " << x << endl;
    #define trace2(x, y)             cerr << #x << ": " << x << " | " << #y << ": " << y << endl;
    #define trace3(x, y, z)          cerr << #x << ": " << x << " | " << #y << ": " << y << " | " << #z << ": " << z << endl;
    #define trace4(a, b, c, d)       cerr << #a << ": " << a << " | " << #b << ": " << b << " | " << #c << ": " << c << " | " << #d << ": " << d << endl;
    #define trace5(a, b, c, d, e)    cerr << #a << ": " << a << " | " << #b << ": " << b << " | " << #c << ": " << c << " | " << #d << ": " << d << " | " << #e << ": " << e << endl;
    #define trace6(a, b, c, d, e, f) cerr << #a << ": " << a << " | " << #b << ": " << b << " | " << #c << ": " << c << " | " << #d << ": " << d << " | " << #e << ": " << e << " | " << #f << ": " << f << endl;

#else

    #define trace1(x)
    #define trace2(x, y)
    #define trace3(x, y, z)
    #define trace4(a, b, c, d)
    #define trace5(a, b, c, d, e)
    #define trace6(a, b, c, d, e, f)

#endif

#define row(x) ((x) / 9)
#define col(x) ((x) % 9)
#define box(x) (3 * (row(x) / 3 ) + (col(x) / 3))

class Genome;

int valid[81], nvalid[81], valid_nums[81][9];
int input[81];
int pop_len, pop_retain, gen;
int restarts, time_taken;
int display_level, print_result;
double mutation_rate, single_crossover_rate, fitter_parent;
Genome *pop;
int con_count[27][10], gen_limit, restart_limit;
int con_mask[27];
int pos[81];
mt19937_64 mrand;
#ifdef RAND_MAX
	#undef RAND_MAX
	#define RAND_MAX mrand.max()
	#define rand mrand
	#define srand(X) mrand.seed(X)
#endif

bool fire(double p) {
    return rand() <= p * RAND_MAX;
}

int randrange(int a, int b) {
    assert(a <= b);
    return rand() % (b - a + 1) + a;
}

int getRandom(int x) {
    int c = 0, nc = nvalid[x] - 1, mask = (con_mask[row(x)] | con_mask[col(x) + 9] | con_mask[box(x) + 18]);
    for(int d = 1; d <= 9; ++d) {
        if((valid[x] & (1 << d)) > 0) {
            if((mask & (1 << d)) == 0) valid_nums[x][c++] = d;
            else valid_nums[x][nc--] = d;
        }
    }
    if(c == 0) c = nvalid[x];
    return valid_nums[x][randrange(0, c - 1)];
}

bool conflict(int *num, int i, int j) {
    return (i != j) && (num[i] == num[j]) && ((row(i) == row(j)) || (col(i) == col(j)) || (box(i) == box(j)));
}

class Genome {
    public:
    int num[81];
    int score;

    Genome() {
        score = -1;
    }

    void single_crossover(Genome g) {
        int x = randrange(1, 79);
        fr(i, x) swap(num[i], g.num[i]);
        score = g.score = -1;
    }

    void mutate() {
        ini(con_mask, 0);
        for(int j = 0; j < 81; ++j) {
            int i = pos[j];
            if(fire(mutation_rate)) {
                num[i] = getRandom(i);
                score = -1;
            }
            con_mask[row(i)] |= (1 << num[i]);
            con_mask[col(i) + 9] |= (1 << num[i]);
            con_mask[box(i) + 18] |= (1 << num[i]);
        }
    }

    void fillrandom() {
        ini(con_mask, 0);
        for(int j = 0; j < 81; ++j) {
            int i = pos[j];
            num[i] = getRandom(i);
            con_mask[row(i)] |= (1 << num[i]);
            con_mask[col(i) + 9] |= (1 << num[i]);
            con_mask[box(i) + 18] |= (1 << num[i]);
        }
        score = -1;
    }

    int get_score() {
        if(score != -1) return score;
        score = 0;
        ini(con_count, 0);
        for(int j = 0; j < 81; ++j) {
            int i = pos[j];
            score += con_count[row(i)][num[i]];
            con_count[row(i)][num[i]] += 1;
            score += con_count[col(i) + 9][num[i]];
            con_count[col(i) + 9][num[i]] += 1;
            score += con_count[box(i) + 18][num[i]];
            con_count[box(i) + 18][num[i]] += 1;
        }

        return score;
    }

    bool operator <(const Genome &g) const {
        assert(score != -1 && g.score != -1);
        return score < g.score;
    }
};

int select() {
    int a = randrange(0, pop_len - 1), b;
    b = a;
    while(b == a) {
        b = randrange(0, pop_len - 1);
    }
    if(pop[a].get_score() > pop[b].get_score()) swap(a, b);
    if(fire(fitter_parent)) return a;
    else return b;
}


void initParams() {
    pop_len = 1000;
    pop_retain = 900;
    mutation_rate = .1;
    single_crossover_rate = .9;
    fitter_parent = 1;
    gen_limit = 1000;
    restart_limit = 5;
    display_level = 1;
    print_result = 1;

    FILE *in = fopen(".gaconfig", "r");
    if(in == NULL) return;
    char var[100];
    int val;
    while(fscanf(in, "%s = %d", var, &val) != EOF) {
        //for(char *c = var; *c; ++c) *c = tolower(*c);
        if(strcasecmp(var, "pop_len") == 0) pop_len = val;
        if(strcasecmp(var, "pop_retain") == 0) pop_retain = val;
        if(strcasecmp(var, "mutation_rate") == 0) mutation_rate = 1.0 * val / 100;;
        if(strcasecmp(var, "single_crossover_rate") == 0) single_crossover_rate = 1.0 * val / 100;
        if(strcasecmp(var, "seed") == 0) srand(val);
        if(strcasecmp(var, "gen_limit") == 0) gen_limit = val;
        if(strcasecmp(var, "restart_limit") == 0) restart_limit = val;
        if(strcasecmp(var, "display_level") == 0) display_level = val;
        if(strcasecmp(var, "print_result") == 0) print_result = val;
        if(strcasecmp(var, "fitter_parent") == 0) fitter_parent = 1.0 * val / 100;
        
    }
    
    fclose(in);
    assert(pop_retain >= 0 && pop_retain < pop_len);
    assert(pop_retain % 2 == 0);
    assert(pop_len >= 10);
    assert(gen_limit >= 10);
    assert(restart_limit >= 0);
    assert(mutation_rate >= 0 && mutation_rate <= 1);
    assert(single_crossover_rate >= 0 && single_crossover_rate <= 1);
    assert(fitter_parent >= 0 && fitter_parent <= 1);
}

void gen_pos() {
    //if(randrange(0, 10)) return;

    int order[9], c = 0;
    for(int i = 0; i < 9; ++i) order[i] = i;
    random_shuffle(order, order + 9);
    int type = randrange(0, 0);
    if(type == 0) {
        for(int i = 0; i < 9; ++i) {
            for(int x = 0; x < 81; ++x) if(row(x) == order[i]) pos[c++] = x;
        }
    } else if(type == 1) {
        for(int i = 0; i < 9; ++i) {
            for(int x = 0; x < 81; ++x) if(col(x) == order[i]) pos[c++] = x;
        }
    } else {
        for(int i = 0; i < 9; ++i) {
            for(int x = 0; x < 81; ++x) if(box(x) == order[i]) pos[c++] = x;
        }
    }

    for(int i = 0; i < 81; ++i) for(int j = 0; j < i; ++j) assert(pos[i] != pos[j]);
}

void disp_progress() {
    //return;
    if(display_level <= 0) return;
    if(display_level <= 1) {
        if(gen == 0) printf("RESTART: %d\n", restarts);
        return;
    }
    printf("I am at gen % 4d with best score % 4d: ", gen, pop[0].get_score());
    fr(i, 81) printf("%d", pop[0].num[i]);
    printf("\n");
    
}

void arrange_min() {
    for(int i = 0; i < pop_len; ++i) if(pop[0].get_score() > pop[i].get_score()) swap(pop[0], pop[i]);
}

void initGA() {
    for(int i = 0; i < 81; ++i) pos[i] = i;
    gen_pos();
    pop = new Genome[pop_len];
    for(int i = 0; i < pop_len; ++i) {
        pop[i].fillrandom();
        if(pop[i].get_score() == 0) {
            swap(pop[i], pop[0]);
            return;
        }
    }
    gen = 0;
    //sort(pop, pop + pop_len);
    nth_element(pop, pop + pop_retain, pop + pop_len);
    arrange_min();
    disp_progress();
}

void wrapGA() {
    delete []pop;
}

void outputStats(int solved) {
    fprintf(stderr, "solved: %d\n", solved);
    fprintf(stderr, "restarts = %d\n", restarts);
    fprintf(stderr, "gen = %d\n", restarts * gen_limit + gen);
    fprintf(stderr, "time = %d\n", time_taken);
    fprintf(stderr, "size = %d\n", pop_len);
    fprintf(stderr, "retain = %d\n", pop_retain);
    fprintf(stderr, "mutation_rate = %lf\n", mutation_rate);
    fprintf(stderr, "single_crossover_rate = %lf\n", single_crossover_rate);
    fprintf(stderr, "fitter_parent = %lf\n", fitter_parent);
    fprintf(stderr, "\n");
}



void nextgen() {
    gen_pos();
    ++gen;
    for(int i = pop_retain; i < pop_len; i += 2) {
        int a = select(), b = select();
        while(a == b) b = select();
        pop[i] = pop[a]; pop[i + 1] = pop[b];
        if(fire(single_crossover_rate)) {
            pop[i].single_crossover(pop[i + 1]);
            pop[i].mutate();
            pop[i + 1].mutate();
        } else {
            pop[i].fillrandom();
            pop[i + 1].fillrandom();
        }

        pop[i].get_score();
        pop[i + 1].get_score();
    }

    nth_element(pop, pop + pop_retain, pop + pop_len);
    arrange_min();
    disp_progress();
}

char inp[100];
void processInput() {
    int full = (1 << 10) - 1;
    fr(i, 81) {
        valid[i] = full;
        nvalid[i] = 0;
        input[i] = inp[i] - '0';
    }

    fr(i, 81) {
        if(input[i]) {
            valid[i] = (1 << input[i]);
            nvalid[i] = 1;
            valid_nums[i][0] = input[i];
            continue;
        }
        fr(j, 81) if(input[j] && (row(i) == row(j) || col(i) == col(j) || box(i) == box(j))) {
            valid[i] &= (full - (1 << input[j]));
        }

        re(d, 1, 10) if(valid[i] & (1 << d)) {
            valid_nums[i][nvalid[i]++] = d;
        }
        assert(nvalid[i]);
    }
}

Genome solve() {
    Genome res;

    while(restarts < restart_limit) {
        initGA();
        while(pop[0].get_score() && gen < gen_limit) {
            nextgen();
        }
        res = pop[0];
        wrapGA();
        if(res.get_score() == 0) break;
        ++restarts;
    }
    return res;
}

int main() {
    initParams();
    scanf("%s", inp);
    processInput();

    clock_t tstart = clock();
    Genome ans = solve();
    time_taken = (clock() - tstart) * 1000 / CLOCKS_PER_SEC;
    if(ans.get_score()) {
        if(print_result >= 1) outputStats(0);
        return 1;
    }
    if(print_result >= 1) printf("RESULT FOUND:\n");
    if(print_result >= 2) {
        fr(i, 81) printf("%c", inp[i]); printf("\n");
        fr(i, 81) printf("%d", ans.num[i]); printf("\n");
    }
    if(print_result >= 3) {
        fr(i, 9) {
            fr(j, 9) printf("%d", ans.num[9 * i + j]);
            printf("\n");
        }
    }

    if(print_result >= 1) outputStats(1);
    
	return 0;
}
