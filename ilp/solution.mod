// PLEASE ONLY CHANGE THIS FILE WHERE INDICATED.

// Number of contestants.
int n = ...;

// Matrix of points.
// Input should satisfy: coefficients >= 0, sum of each row == 100.
int p[1..n][1..n] = ...;

// Range of contestants/slots.
range N = 1..n;

// Define here your decision variables and
// any other auxiliary program variables you need.
// You can run an execute block if needed.

// Matches. 
// white: index identifying the player playing white in this match
// black: index identifying the player playing black in this match
// slot:  Time slot when this game is played
dvar boolean match[white in N][black in N][slot in N];

dvar int+ totalpoints;
dvar int+ points[x in N];

maximize totalpoints; // Write here the objective function.

constraint no_overcommit[N];
constraint no_self_match[N];
constraint input_check[N];
constraint number_of_matches[N];

constraint x_vs_y[N][N];
constraint matches_per_day[N];

subject to {

    // Write here the constraints.

    // Participant input OK
    //forall(participant in N) {
    //    input_check[participant]:
    //    sum(slot in N)
    //        p[participant][slot] == 100;
    //}
    
    //// //// //// //// ////
    // REVISAR


	// Esto realmente ya no hace falta al haber arreglado lo de each player plays vs each other
	// exactly once..

	// No hace falta pero es util, speedup
	// You cant play vs yourself!
	forall(x in N) {
	    no_self_match[x]:
	    forall(k in N) {
	        match[x][x][k] == 0;
	    }
	}

    //// //// //// //// ////
		
    // Every player plays vs each other exactly 
    forall(x in N) {
	forall(y in N) {
	    x_vs_y[x][y]:
	    (sum(k in N) match[x][y][k] + sum(k in N) match[y][x][k]) * (x - y) == (x - y);
	}
    }


    // Number of matches per round
    forall(k in N) {
	number_of_matches[k]:
	sum(x in N) (
	    sum(y in N) (
		match[x][y][k]
	    )
	) == (n - 1)/2;
    }

    // A player can only play up to 1 game per round
    forall(k in N) {
	matches_per_day[k]:
	forall(x in N) {
	    sum(y in N) (match[x][y][k] + match[y][x][k]) <= 1;
	}
    }

    totalpoints <= sum(k in N) (
	sum(x in N) (
	    p[x][k] * (1 - (sum(y in N) (match[x][y][k] + match[y][x][k])))
	)
    );
    
    //forall(k in N) {
    //    forall(x in N) {
    //        points[x] >= (p[x][k] * (1 - (sum(y in N) (match[x][y][k] + match[y][x][k])))) ;
    //    }
    //}
	    
    //totalpoints <= 10;
    //totalpoints >= sum(p in N) points[p];
}

// You can run an execute block if needed.

execute {
    for(var player = 1; player <= n; player++) {
	for(var k = 1; k <= n; k++) {
	    var match_in_k = 0;
	    for(var other = 1; other <= n; other++) {
		if(match[player][other][k] | match[other][player][k])
		    match_in_k++;
	    }

	    if(match_in_k == 0) {
		writeln(player + " is free in round " + k);
	    } else {
		//writeln(player + " is busy in round " + k + ". " + match_in_k);
	    }
	}
    }


    for(var s = 1; s <= n; s++) {
	writeln("Slot " + s);
	for(var w = 1; w <= n; w++) {
	    for(var b = 1; b <= n; b++) {
		if(match[w][b][s] == 1) {
		    writeln(w + " white, " + b + " black")		
		}
	    }
	}

	writeln();
    }
}
