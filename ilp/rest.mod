// Number of contestants.
int n = ...;

// Matrix of points.
// Input should satisfy: coefficients >= 0, sum of each row == 100.
int p[1..n][1..n] = ...;

// Range of contestants/slots.
range N = 1..n;



dvar boolean free[slot in N][player in N];


// Objective function
dvar int+ score;
maximize score; 

subject to {

    // only 1 player rests in each day
    forall(day in N) 
	(sum(player in N) free[day][player]) == 1;

    forall(player in N)
	sum(day in N) free[day][player] == 1;

    score <= sum(k in N) (
	sum(x in N) (
	    p[x][k] * free[k][x] 
	)
    );
}

execute {
}
