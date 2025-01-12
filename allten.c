// TODO: options

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

static bool g_only_show_solvable = false; // when true: only showw whether or not sol exists
static bool g_show_all = false;           // when true: all solutions are given for each target

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

bool check_answer(NumDen* res, int target) {
	return res->den != 0 && res->num == target * res->den;
}

void print_solution(const char* format, NumDen* res,
		int a, int b, int c, int d, Operation op1, Operation op2, Operation op3) {
	if (res->den != 0 && res->num % res->den == 0) {
		char totformat[80];
		sprintf(totformat, "%%4d = %s\n", format);
		printf(totformat, res->num/res->den, a, opstr[op1], b, opstr[op2], c, opstr[op3], d);
	}
	else
		fprintf(stderr, "Invalid solution given to print_solution()\n");
}

bool solve(int n[], int target) {
	// There are 5 basic forms
	// (a@b)@(c@d)
	// ((a@b)@c)@d
	// (a@(b@c))@d
	// a@((b@c)@d)
	// a@(b@(c@d))
	// @ is operator. This can only be concat when applied to original numbers, not to intermed results
	NumDen nd[4];
	NumDen res1, res2, res3;
	bool solvable = false;

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
					if (check_answer(&res3, target)) {
						solvable = true;
						if (!g_only_show_solvable)
							print_solution("(%d%s%d)%s(%d%s%d)", &res3,
								nd[0].num, nd[1].num, nd[2].num, nd[3].num, op1, op3, op2);
						if (!g_show_all)
							return true;
					}
					if (op2 != CONCAT) {
						// ((a@b)@c)@d
						(*ops[op1])(&nd[0], &nd[1], &res1);
						(*ops[op2])(&res1, &nd[2], &res2);
						(*ops[op3])(&res2, &nd[3], &res3);
						if (check_answer(&res3, target)) {
							solvable = true;
							if (!g_only_show_solvable)
								print_solution("((%d%s%d)%s%d)%s%d", &res3,
									nd[0].num, nd[1].num, nd[2].num, nd[3].num, op1, op2, op3);
							if (!g_show_all)
								return true;
						}
						// (a@(b@c))@d
						(*ops[op1])(&nd[1], &nd[2], &res1);
						(*ops[op2])(&nd[0], &res1, &res2);
						(*ops[op3])(&res2, &nd[3], &res3);
						if (check_answer(&res3, target)) {
							solvable = true;
							if (!g_only_show_solvable)
								print_solution("(%d%s(%d%s%d))%s%d", &res3,
									nd[0].num, nd[1].num, nd[2].num, nd[3].num, op2, op1, op3);
							if (!g_show_all)
								return true;
						}
						// a@((b@c)@d)
						(*ops[op1])(&nd[1], &nd[2], &res1);
						(*ops[op2])(&res1, &nd[3], &res2);
						(*ops[op3])(&nd[0], &res2, &res3);
						if (check_answer(&res3, target)) {
							solvable = true;
							if (!g_only_show_solvable)
								print_solution("%d%s((%d%s%d)%s%d)", &res3,
									nd[0].num, nd[1].num, nd[2].num, nd[3].num, op3, op1, op2);
							if (!g_show_all)
								return true;
						}
						// a@(b@(c@d))
						(*ops[op1])(&nd[2], &nd[3], &res1);
						(*ops[op2])(&nd[1], &res1, &res2);
						(*ops[op3])(&nd[0], &res2, &res3);
						if (check_answer(&res3, target)) {
							solvable = true;
							if (!g_only_show_solvable)
								print_solution("%d%s(%d%s(%d%s%d))", &res3,
									nd[0].num, nd[1].num, nd[2].num, nd[3].num, op3, op2, op1);
							if (!g_show_all)
								return true;
						}
					}
				}
			}
		}
	} while (next_perm(idx, 4));
	return solvable;
}

void usage(const char* progname) {
	printf("Usage: %s [options] nr1 nr2 nr3 nr4\n", progname);
	printf("\nIf no target given, all targets 1..10 are attempted\n");
	printf("Options:\n");
	printf("  s:      only shows whether or not the problem is solvable\n");
	printf("  a:      shows ALL solutions (tends to produce a lot of output\n");
	printf("  t<n>:   instead of finding all targets 1..10, finds only give target\n");
	printf("  o<ops>: allowed operations. Default: +-*/c\n");
	printf("            e.g.: o+-/*c\n");
	printf("            operator c is the concat operator. For example c(a,b) = 10*a + b\n");
	printf("            So c(3,1) = 31\n");
}

int main(int argc, char* argv[]) {
	// extract nrs from command line
	int nrs_read = 0;
	int x[4]; // the numbers
	bool single_target = false;
	int target_given = 0;

	for (int argidx = 1; argidx < argc; ++argidx) {
		char* arg = argv[argidx];
		if (isdigit(arg[0]))
			x[nrs_read++] = atoi(arg);
		else { // arg starts with non-digit
			while (*arg) {
				switch (*arg) {
					case 's':
						g_only_show_solvable = true;
						break;
					case 'a':
						g_show_all = true;
						break;
					case 't':
						++arg;
						while (isdigit(*arg)){
							single_target = true;
							target_given = 10 * target_given + *arg - '0';
							++arg;
						}
						--arg; // counter ++arg later
						break;
					case 'o':
						fprintf(stderr, "Option 'o' not supported yet\n");
						break;
					default:
						break; // default is to ignore char
				}
				++arg;
			}
		}
	}

	if (nrs_read < 4) {
		usage(argv[0]);
		return -1;
	}

	// Solve for the numbers
	for (int target = 1; target <= 10; ++target) {
		if (!single_target || target == target_given) {
			bool solvable = solve(x, target);
			if (g_only_show_solvable)
				printf("%4d: %ssolvable\n", target, (solvable?"":"un"));
		}
	}

	return 0;
}

