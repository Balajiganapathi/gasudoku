/*
 * Program to solve sudoku using Genetic Algorithm.
 * Input is a line with 81 characters of sudoku in row major order with 0 denoting blank cells
 * Output is the solution (if found) and stats
 * The solution is printed to stdout and stats to stderr
 * */

#include "common.h"

// Macros to extract the number of a region from the cell index. All are zero based.
#define row(x) ((x) / 9)
#define col(x) ((x) % 9)
#define box(x) (3 * (row(x) / 3 ) + (col(x) / 3))
int input[81];          // The input puzzle. Zero represents blank cell other have numbers 1-9.
int valid[81];          // A bitmask with one at bit b representing that b does not 
						// conflict with any of the input numbers
int nvalid[81];         // The number of valid digits (the no. of ones in valid[x]
int valid_nums[81][9];  // The positions valid_nums[x][0] ... valid_nums[x][nvalid[x] - 1] 
						// contain all valid numbers
int pop_retain;         // Elitism. The amount of genomes to retain when building next generation
int gen;                // The current generation no. Starts form 0
int gen_limit;          // Number of times a new generation is created before a restart is done
int restarts;           // The number of restarts
int restart_limit;      // The number of times restart happens before the program gives up
int time_taken;         // Time taken by the program to solve the sudoku in milliseconds
double mutation_rate;   // The probability of a row being mutated. See mutate function 
						// of Genome class for more details. Usually (0.01, 0.2)
double single_crossover_rate;   // The probability of a crossover being performed between 
						// two genomes. See crossover and next_gen code for details
double fitter_parent;   // The probability of a fitter genome being selected. 
						// See select for more details.
int display_level;      // Controls amount of data to display, higher no. means more data 
						// is printed. Useful for debugging
int print_result;       // Controls amount of result being printed to stdout. 
						// Higher no.m means more data is printed
int pop_len;            // The number of genomes in a population
Genome *pop;            // An array of length pop_len with each element containing a genome. 
						// This is initialized in init_ga
int con_count[27][10];  // Keeps a count of the number of times a particular number appears 
						// in the given region. The region mapping is same as that of con_mask
int con_mask[27];       // Conflict mask. Similar to valid[x], but accumulates all masks for a 
						// particular region. 0-8 -> rows 0 to 8, 9-17 -> col 0 to 8, 
						// 18-26 -> box 0 to 8. Used for better selection of a random number 
						// to fill a particular cell.

bool run_once = true;

void initMetaParams(int params[])
{
	for (int i = 0; i < 4; i++) 
		cout << params[i] << " ";
	cout << endl;
	srand(params[0]);
    pop_retain            = params[1];
    mutation_rate         = params[2]/1000.0;
    single_crossover_rate = params[3]/1000.0;
    gen                   = 0;
    restarts              = 0;
	memset(con_count, 0, 27*10*sizeof(int));
	memset(con_mask, 0, 27*sizeof(int));
	memset(input, 0, 81*sizeof(int));
	memset(valid, 0, 81*sizeof(int));
	memset(nvalid, 0, 81*sizeof(int));

}

// Returns true with probability p
bool fire(double p) {
    return rand() <= p * RAND_MAX;
}

// Returns a number choosen uniformly at random between a and b inclusive
int randrange(int a, int b) {
    assert(a <= b);
    return rand() % (b - a + 1) + a;
}

// Optimization hack
// Rearranges the numbers in valid_nums such that the numbers whose bit 
// is set in the mask is placed at the begining
// Returns the no. of such elements
int rearrange(int x, int mask) {
    int c = 0, nc = nvalid[x] - 1;

    for(int d = 1; d <= 9; ++d) {
        if((valid[x] & (1 << d)) > 0) {
            if((mask & (1 << d)) == 0) {
                valid_nums[x][c++] = d;
            } else {
                valid_nums[x][nc--] = d;
            }
        }
    }

    return c;
}

// This function is the primary reason for this program to be so succesful
// It chooses a number to replace the current number in num[x]. It uses 
// the information stored in con_mask to cleverly choose a number with 
// minimum conflict. If all possible numbers conflict, it chooses a random
// number from valid_nums
int getRandom(int x) {
    int c;
    int masks[] = {
        (con_mask[row(x)] | con_mask[col(x) + 9] | con_mask[box(x) + 18]),
        (con_mask[row(x)] | con_mask[box(x) + 18]),
        (con_mask[row(x)] | con_mask[col(x) + 9]),
        (con_mask[row(x)])};
    
    fr(i, 4) {
        c = rearrange(x, masks[i]);
        if(c) break;
    }
    if(c == 0) {
        c = nvalid[x];
        /*
        int d;
        do {
            d = randrange(1, 9);
        } while(con_mask[row(x)] & (1 << d));
        return d;
        */
    }
        
    return valid_nums[x][randrange(0, c - 1)];
}

// returns true if cell i and cell j are different and in teh same region
bool conflict(int *num, int i, int j) {
    return (i != j) && (num[i] == num[j]) && ((row(i) == row(j)) || (col(i) == col(j)) || (box(i) == box(j)));
}

//class Genome {

Genome::Genome() {
    score = -1; // Make cache dirty
}

// Returns true if all rows of the solutions are valid.	
bool Genome::row_consistent() {
    fr(i, 81) fr(j, 81) if(i != j && row(i) == row(j) && num[i] == num[j]) return false;
    return true;
}

// Performs a simple single point crossover of this genome with the input genome p
// To do this, it chooses a row boundary x uniformly at random, and swaps all the values till that point
void Genome::single_crossover(Genome g) {
    int x = randrange(1, 8) * 9;
    fr(i, x) swap(num[i], g.num[i]);
    score = g.score = -1;
}

// This function mutates a row with probability mutation_rate
// When a row is selected for mutation, a cell in that row is choosen 
// uniformly at random and the cells starting from the chosen cell to 
// the end of row are filled randomly.
void Genome::mutate() {
    int from;
    ini(con_mask, 0);
    for(int i = 0; i < 81; ++i) {
        if(col(i) == 0) { // Is this a new row?
            from = oo;
            if(fire(mutation_rate)) {// Mutate?
                from = i + randrange(0, 8);; // Select a random cell in the row
                score = -1; // Cache violated;
            }
        }
		// gets a new value for cell i if it was selected for mutation
        if(i >= from) num[i] = getRandom(i);	
		
		// Update the conflict mask by settin the num[i]th bit in each of the 3 regions
		con_mask[row(i)] |= (1 << num[i]);
        con_mask[col(i) + 9] |= (1 << num[i]);
        con_mask[box(i) + 18] |= (1 << num[i]);
    }
}

// Randomly fills all the cells. It also updates the con_mask to make 
// sure only valid numbers are chosen as far as possible
void Genome::fillrandom() {
    ini(con_mask, 0);
    for(int i = 0; i < 81; ++i) {
        num[i] = getRandom(i);
        assert(num[i] >= 0);
        con_mask[row(i)] |= (1 << num[i]);
        con_mask[col(i) + 9] |= (1 << num[i]);
        con_mask[box(i) + 18] |= (1 << num[i]);
    }
    score = -1; // Cache violated
}

// Calculates the score. The score (or fitness) is the number of pair of 
// cells that conflict. A pair of cell is said to conflict if they both 
// have the same number and are in the same regoin.
//
// Optimization hack: Instead of O(n^2) looping through all pair of cells,
// it loops through all cells once and updates the con_count array for 
// each region. Note that this counts all pairs of cells which appear in 
// the same box as well as same row/col twice. But it doesn't matter as 
// the final decreases as we move towards the solution which is what 
// matters. In fact counting such conflicts multiple time is seen to 
// converge faster though the reason is not clear.
int Genome::get_score() {
    // If the cache is not violated then return the cached value
    if(score != -1) return score;
    score = 0;
    ini(con_count, 0);
    for(int i = 0; i < 81; ++i) {
        assert(num[i] >= 1 && num[i] < 10);
        score += con_count[row(i)][num[i]];
        con_count[row(i)][num[i]] += 1;
        score += con_count[col(i) + 9][num[i]];
        con_count[col(i) + 9][num[i]] += 1;
        score += con_count[box(i) + 18][num[i]];
        con_count[box(i) + 18][num[i]] += 1;
    }

    return score;
}

// Compare two genomes based on their score. 
// The cache must be valid at this stage
bool Genome::operator <(const Genome &g) const {
    assert(score != -1 && g.score != -1);
    return score < g.score;
}
//};

// Implements tournament selection. It works as follows:
// Select two random genome, with probability fitter_parent return 
// the fitter of the two individual.
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

// Initializes all the parameters. See the declaration of each parameter 
// for explanation. It also reads from a file called '.gaconfig' (if 
// present) and takes the values from there instead. The configuration 
// file *MUST* follow ecactly this syntax:
// <variable> = <value>
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
        for(char *c = var; *c; ++c) *c = tolower(*c);
        if(strcmp(var, "pop_len") == 0) pop_len = val;
        if(strcmp(var, "pop_retain") == 0) pop_retain = val;
        if(strcmp(var, "mutation_rate") == 0) mutation_rate = 1.0 * val / 100;;
        if(strcmp(var, "single_crossover_rate") == 0) single_crossover_rate = 1.0 * val / 100;
        if(strcmp(var, "seed") == 0) srand(val);
        if(strcmp(var, "gen_limit") == 0) gen_limit = val;
        if(strcmp(var, "restart_limit") == 0) restart_limit = val;
        if(strcmp(var, "display_level") == 0) display_level = val;
        if(strcmp(var, "print_result") == 0) print_result = val;
        if(strcmp(var, "fitter_parent") == 0) fitter_parent = 1.0 * val / 100;
        
    }
    
    fclose(in);

    // Make sure the values set are in their proper range
    assert(pop_retain >= 0 && pop_retain < pop_len);
    assert(pop_retain % 2 == 0);
    assert(pop_len >= 10);
    assert(gen_limit >= 10);
    assert(restart_limit >= 0);
    assert(mutation_rate >= 0 && mutation_rate <= 1);
    assert(single_crossover_rate >= 0 && single_crossover_rate <= 1);
    assert(fitter_parent >= 0 && fitter_parent <= 1);
}

// Display the progress of the algo so far
// Amount of display depends on the variable display_level
// 0 -> no display
// 1 -> only display restarts
// > 1 -> Displays stat for each generation (Warning: Huge amount of o/p data)
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

// Make sure pop[0] is the best genome.
void arrange_min() {
    for(int i = 0; i < pop_len; ++i) if(pop[0].get_score() > pop[i].get_score()) swap(pop[0], pop[i]);
}

// Prepares things for starting the GA
// 1. Allocate memory for the population genomes
// 2. Fill all genomes with random values
// 3. Prepare for creating next generation by retaining the top 
// pop_retain genomes from current generation
void initGA() {
	if (run_once)
	{	
		pop = new Genome[pop_len]();
		run_once = false;
	}
    for(int i = 0; i < pop_len; ++i) {
        pop[i].fillrandom();
        //assert(pop[i].row_consistent());
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

// Called after running GA
// Deallocates all allocated storage
void wrapGA() {
	if (pop)
		delete []pop;
	pop = NULL;
}

// Output the statistice of this run
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

// Generate the next generation of the population. It works as follows:
// 1. Retain the top pop_retain of the population
// 2. Replace the remaining elements two at a time as follows:
//      a. Select two of the genomes for replacement using tournament selection
//      b. With probability single_crossover apply the single crossover operator to them 
//      c. With probability (1 - single_crossover) replace the genomes by entirely random genome
//      d. Calculate the score
// 3. Prepare the population for generating next generation
void nextgen() {
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

char inp[100];// The input puzzle in string form

// Preprocesses the input for easy usage. It takes a line of puzzle from 
// stdin, converts them into integers and stores them in input[].
// Then for each cell, it calculates all possible digits that can appear 
// there and keeps track of it in valid, valid_nums and nvalid.
// This is used by getRandom function to choose only valid digits.
void processInput() {
    int full = (1 << 10) - 2;
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

// Given an input sudoku in input[], this function keeps generating 
// populations until one with zero score is found or till the restart 
// limit is reached. In either case it returns the best member of all 
// the population in this restart.
// The caller should check whether the score of the returned Genome 
// is zero or not to determine if it is a solution.
Genome solve() {
    Genome res;

    while(restarts < restart_limit) {
        initGA();
        while(pop[0].get_score() && gen < gen_limit) {
            nextgen();
        }
        res = pop[0];
      
        if(res.get_score() == 0) break;
        ++restarts;
    }
    return res;
}

int main(int argc, char* argv[]) {
    scanf("%s", inp);
	if (argc > 1)
		return metaGA(argc, argv);

    initParams();
    processInput();
	
	// Keep track of the time. Stores the return of clock in tstart.
    clock_t tstart = clock();
    Genome ans = solve();	// Try to solve the puzzle
	// Calculate the time taken by solve by subtracting tstart from current time
    time_taken = (clock() - tstart) * 1000 / CLOCKS_PER_SEC;
    if(ans.get_score()) {	
		// Is the best output valid or not? 
		// If > 0 then it is not a valid solution. 
		// If so, print stats and exit
        if(print_result >= 1) outputStats(0);
        return 1;
    }
	// Print the output based on the print_result parameter
    
	if(print_result >= 1) printf("RESULT FOUND:\n");
    
	// Print the input and answer in row major form for easy comparision
	if(print_result >= 2) {
        fr(i, 81) printf("%c", inp[i]); printf("\n");
        fr(i, 81) printf("%d", ans.num[i]); printf("\n");
    }

    // Print the output in standard sudoku form.
    if(print_result >= 3) {
        fr(i, 9) {
            fr(j, 9) printf("%d", ans.num[9 * i + j]);
            printf("\n");
        }
    }

    if(print_result >= 1) outputStats(1);

	return 0;
}
