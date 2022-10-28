// Number of contestants.
int n = ...;

// Matrix of points.
// Input should satisfy: coefficients >= 0, sum of each row == 100.
int p[1..n][1..n] = ...;

// Range of contestants/slots.
range N = 1..n;



// Matches. 
// white: index identifying the player playing white in this match
// black: index identifying the player playing black in this match
// slot:  Time slot when this game is played
dvar boolean match[white in N][black in N][slot in N];



// Debug
constraint no_overcommit[N];
constraint no_self_match[N];
constraint input_check[N];
constraint number_of_matches[N];
constraint x_vs_y[N][N];
constraint matches_per_day[N];


// Objective function
dvar int+ score;
maximize score; 

subject to {

// Participant input OK
//forall(participant in N) {
//    input_check[participant]:
//    sum(slot in N)
//        p[participant][slot] == 100;
//}
    
/////////////////////////////////////////////////////
// Every player plays vs each other (no games with themselves)
/////////////////////////////////////////////////////

    // Every player plays vs each other exactly once
    forall(x in N) {
	forall(y in N : x != y) {
	    x_vs_y[x][y]:
	    (sum(k in N) match[x][y][k] + sum(k in N) match[y][x][k]) == 1;
	}
    }

    // REVISAR
    // No hace falta pero es util, speedup¿?¿?

    // You cant play vs yourself!
    forall(x in N) {
        no_self_match[x]:
        forall(k in N) {
            match[x][x][k] == 0;
        }
    }


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////



/////////////////////////////////////////////////////
// Number of matches per round (per player)
/////////////////////////////////////////////////////


    // A player can only play up to 1 game per round
    forall(k in N) {
	matches_per_day[k]:
	forall(x in N) {
	    sum(y in N) (match[x][y][k] + match[y][x][k]) <= 1;
	}
    }

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////



/////////////////////////////////////////////////////
// Number of matches per round
/////////////////////////////////////////////////////

    // Number of matches per round
    forall(k in N) {
	number_of_matches[k]:
	sum(x in N) (
	    sum(y in N) (
		match[x][y][k]
	    )
	) == (n - 1)/2;
    }

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////




/////////////////////////////////////////////////////
// Number of matches as white / black
/////////////////////////////////////////////////////


// TODO: One of them is enough, check if extra one speeds or slows things down

    forall(x in N) {
        (sum(y in N, k in N)
            match[x][y][k]) == (n - 1)/2;
    }

    //forall(x in N) {
    //    (sum(y in N, k in N)
    //        match[y][x][k]) == (n - 1)/2;
    //}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////



    score <= sum(k in N) (
	sum(x in N) (
	    p[x][k] * (1 - (sum(y in N) (match[x][y][k] + match[y][x][k])))
	)
    );
}

execute {
    for(var player = 1; player <= n; player++) {
	var white_matches = 0;
	var black_matches = 0;
	for(var k = 1; k <= n; k++) {
	    var match_in_k = 0;
	    for(var other = 1; other <= n; other++) {
		if(match[player][other][k] | match[other][player][k])
		    match_in_k++;

		if(match[player][other][k] == 1)
		    white_matches++;
		if(match[other][player][k] == 1)
		    black_matches++;
	    }

	    if(match_in_k == 0) {
		writeln(player + " is free in round " + k);
	    } else {
		//writeln(player + " is busy in round " + k + ". " + match_in_k);
	    }
	}
	writeln(player + " plays " + white_matches + " white matches, and " + black_matches + " black matches")
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
