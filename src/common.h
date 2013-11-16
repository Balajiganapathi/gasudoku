#ifndef __COMMON_H
#define __COMMON_H
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;
typedef pair<int,int> pi;
typedef vector<string> vs;

// Basic macros
#define ini(a, v)   memset(a, v, sizeof(a))
#define re(i,s,n)  	for(int i=s;i<(n);++i)
#define rep(i,s,n)  for(int i=s;i<=(n);++i)
#define fr(i,n)     re(i,0,n)
#define tr(i,x)     for(typeof(x.begin()) i=x.begin();i!=x.end();++i)
#define sz(x)       (int)(x.size())

const int oo = 2000000009;

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

// Represents a potential solution to the given input sudoku
class Genome
{
    public:
    int num[81];
    int score;

	Genome();
	bool row_consistent();
	void single_crossover(Genome g);
	void mutate();
	void fillrandom();
	int get_score();
	bool operator <(const Genome &g) const;
};

bool fire(double);
int randrange(int , int );
void initParams();
void processInput();
Genome solve();

int metaGA(int, char* []);
void initMetaParams(int [], int, int seed = 0);
#endif
