// TODO: options

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct NumDen {
	int num;
	int den;
} NumDen;

typedef enum {
	// Commutative ops
	ADD = 0,
	MUL,

	// Non-comm
	SUB,
	DIV,

	// can only be used on starting nrs
	CONCAT,

	NR_OPS
} Operation;

const char* opstr[5] = {
	" + ",
	" * ",
	" - ",
	" / ",
	""
};

typedef void (*op_fn)(NumDen* nd1, NumDen* nd2, NumDen* res);

void set(NumDen* nd, int n, int d) {
	nd->num = n;
	nd->den = d;
}

void times(NumDen* nd1, NumDen* nd2, NumDen* res) {
	res->num = nd1->num * nd2->num;
	res->den = nd1->den * nd2->den;
}

void divby(NumDen* nd1, NumDen* nd2, NumDen* res) {
	int n = nd1->num * nd2->den;
	int d = nd1->den * nd2->num;
	res->num = n;
	res->den = d;
}

void plus(NumDen* nd1, NumDen* nd2, NumDen* res) {
	res->num = nd1->num * nd2->den + nd2->num * nd1->den;
	res->den = nd1->den * nd2->den;
}

void minus(NumDen* nd1, NumDen* nd2, NumDen* res) {
	res->num = nd1->num * nd2->den - nd2->num * nd1->den;
	res->den = nd1->den * nd2->den;
}

void concat(NumDen* nd1, NumDen* nd2, NumDen* res) {
	if (nd1->den != 1 || nd2->den != 1 || nd1->num >= 10 || nd2->num >= 10) {
		fprintf(stderr, "concat called with invalid params\n");
		return;
	}
	res->num = nd1->num*10 + nd2->num;
	res->den = 1;
}

op_fn ops[] = {plus, times, minus, divby, concat};

bool next_perm(int a[], int len) {
	// returns false when no next permutation exists
	// step 1: from right to left, find first element higher than its right neighbor: pivot = a[idxPivot]
	// step 2: sort elements to right of idxPivot (invert order in this case!)
	// step 3: from idxPivot upward, find first element > pivot: idxSwap
	// step 4: swap pivot with a[idxSwap]
	int idxPivot;
	for (idxPivot = len - 2; idxPivot >= 0; --idxPivot) {
		if (a[idxPivot] < a[idxPivot + 1]) {
			// invert order of elements idxPivot+1 ... len-1
			int t;
			int ii = idxPivot + 1;
			int jj = len - 1;
			while (jj > ii) {
				t = a[jj];
				a[jj--] = a[ii];
				a[ii++] = t;
			}
			// from idxPivot upward, find first element > pivot: ii
			for (ii = idxPivot + 1; a[ii] < a[idxPivot]; ++ii);
			// swap a[ii] with a[idxPivot]
			t = a[ii];
			a[ii] = a[idxPivot];
			a[idxPivot] = t;
			break;
		}
	}
	return (idxPivot >= 0);
}

bool check_answer(NumDen* res, bool answers[], const char* format,
		int a, int b, int c, int d, Operation op1, Operation op2, Operation op3) {
	char totformat[80];
	int ans = (res->den != 0 && res->num % res->den == 0) ? res->num/res->den : 0;
	if (ans >= 1 && ans <= 10) {
		if (!answers[ans - 1]) {
			answers[ans -  1] = true;
			sprintf(totformat, "%%2d = %s\n", format);
			printf(totformat, ans, a, opstr[op1], b, opstr[op2], c, opstr[op3], d);
		}
	}
	return ans != 0;
}

void usage(const char* progname) {
	printf("Usage: %s [options] nr1 nr2 nr3 nr4 [target]\n", progname);
	printf("\nIf no target given, all targets 1..10 are attempted\n");
	printf("Options:\n");
	printf("  [-]s:      only shows if the problem is solvable\n");
	printf("  [-]a:      shows ALL solutions (tends to produce a lot of output\n");
	printf("  [-]o<ops>: allowed operations\n");
	printf("               e.g.: o+-/*c\n");
	printf("               operator c is the concat operator. For example c(a,b) = 10*a + b\n");
	printf("               So c(3,1) = 31\n");
}

void solve(int n[]) {
	// There are 5 basic forms
	// (a@b)@(c@d)
	// ((a@b)@c)@d
	// (a@(b@c))@d
	// a@((b@c)@d)
	// a@(b@(c@d))
	// @ is operator. This can only be concat when applied to original numbers, not to intermed results
	NumDen nd[4];
	NumDen res1, res2, res3;

	bool answers[10] = {false};
	int idx[4] = {0, 1, 2, 3}; // idx into n[]
	do { // loops over permutations of idx[]
		// init numdens
		for (int ii = 0; ii < 4; ++ii)
			set(&nd[ii], n[idx[ii]], 1);
		for (Operation op1 = ADD; op1 <= CONCAT; ++op1) {
			for (Operation op2 = ADD; op2 <= CONCAT; ++op2) {
				for (Operation op3 = ADD; op3 < CONCAT; ++op3) { // op3 can never be concat
					// (a@b)@(c@d)
					(*ops[op1])(&nd[0], &nd[1], &res1);
					(*ops[op2])(&nd[2], &nd[3], &res2);
					(*ops[op3])(&res1, &res2, &res3);
					check_answer(&res3, answers, "(%d%s%d)%s(%d%s%d)",
					nd[0].num, nd[1].num, nd[2].num, nd[3].num, op1, op3, op2);
					if (op2 != CONCAT) {
						// ((a@b)@c)@d
						(*ops[op1])(&nd[0], &nd[1], &res1);
						(*ops[op2])(&res1, &nd[2], &res2);
						(*ops[op3])(&res2, &nd[3], &res3);
						check_answer(&res3, answers, "((%d%s%d)%s%d)%s%d",
						nd[0].num, nd[1].num, nd[2].num, nd[3].num, op1, op2, op3);
						// (a@(b@c))@d
						(*ops[op1])(&nd[1], &nd[2], &res1);
						(*ops[op2])(&nd[0], &res1, &res2);
						(*ops[op3])(&res2, &nd[3], &res3);
						check_answer(&res3, answers, "(%d%s(%d%s%d))%s%d",
						nd[0].num, nd[1].num, nd[2].num, nd[3].num, op2, op1, op3);
						// a@((b@c)@d)
						(*ops[op1])(&nd[1], &nd[2], &res1);
						(*ops[op2])(&res1, &nd[3], &res2);
						(*ops[op3])(&nd[0], &res2, &res3);
						check_answer(&res3, answers, "%d%s((%d%s%d)%s%d)",
						nd[0].num, nd[1].num, nd[2].num, nd[3].num, op3, op1, op2);
						// a@(b@(c@d))
						(*ops[op1])(&nd[2], &nd[3], &res1);
						(*ops[op2])(&nd[1], &res1, &res2);
						(*ops[op3])(&nd[0], &res2, &res3);
						check_answer(&res3, answers, "%d%s(%d%s(%d%s%d))",
						nd[0].num, nd[1].num, nd[2].num, nd[3].num, op3, op2, op1);
					}
				}
			}
		}
	} while (next_perm(idx, 4));
}

int main(int argc, char* argv[]) {
	// extract nrs from command line
	if (argc < 5) {
		printf("Give the four numbers as arguments\n");
		return -1;
	}
	int x[4];
	for (int ii = 0; ii < 4; ++ii)
		x[ii] = atoi(argv[1 + ii]);
	// Solve for the numbers
	solve(x);

	return 0;
}

