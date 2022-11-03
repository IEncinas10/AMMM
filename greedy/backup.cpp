/*void create_matchups() {
	const uint64_t days = num_players;
	// we assign the ID to the players
	for (uint64_t player = 0; player < num_players; player++) {
	    players[player].playerID = player;
	}

	for (uint64_t day = 0; day < days; day++) {
	    fmt::print("DAY {}\n", day);

	    // sort by ascending points the players each day
	    fmt::print("Ordered vector: ");

	    // Copy of the players vector. We do this in order to update next_color correctly
	    // we could also create another vector that separately tracks this and index it by playerID
	    std::vector<Player> players_day(players);
	    std::sort(players_day.begin(), players_day.end(), [day](const Player &x, const Player &y) {
		if (x.hasRested && !y.hasRested)
		    return true;

		if (!x.hasRested && y.hasRested)
		    return false;

		return x.points_per_day[day] < y.points_per_day[day];
	    });

	    for (int i = 0; i < num_players; i++) {
		fmt::print("{} ", players_day[i].playerID);
	    }
	    fmt::print("\n");

	    std::vector<Player> players_white;
	    std::copy_if(players_day.begin(), players_day.end(), std::back_inserter(players_white),
			 Player::can_play_white);

	    std::vector<Player> players_black;
	    std::copy_if(players_day.begin(), players_day.end(), std::back_inserter(players_black),
			 Player::can_play_black);

	    create_matches_round(players_white, players_black, day);
	}
    }*/

    // Aqui comprobamos: partidas NO repetidas, los jugadores juegan 50% blancas, 50% negras
    bool create_match(std::vector<Player> &players_white, std::vector<Player> &players_black, uint64_t day) {
	for (uint64_t i = 0; i < players_white.size(); i++) {
	    const auto &w = players_white[i];
	    for (uint64_t j = 0; j < players_black.size(); j++) {
		const auto &b = players_black[j];

		if (w.playerID == b.playerID)
		    continue;

		Game g{w.playerID, b.playerID};

		// Ignore repeated matches
		if (!is_new_game(g)) {
		    fmt::print("Repeated game {} - {}\n", w.playerID, b.playerID);
		    continue;
		}

		fmt::print("New game {} - {}\n", w.playerID, b.playerID);
		// w.playerID vs b.playerID can't be created again
		games.push_back(g);

		// Update next color for both players
		players[w.playerID].games_white++;
		players[b.playerID].games_black++;
		matches.insert({day, w.playerID, b.playerID});

		// Remove players from today's list, as they already have a match
		for (const auto &p : players_white) {
		    fmt::print("{}, ", p.playerID);
		}
		fmt::print("\n");
		for (const auto &p : players_black) {
		    fmt::print("{}, ", p.playerID);
		}
		fmt::print("\n");

		player_id whiteID = w.playerID;
		player_id blackID = b.playerID;
		players_white.erase(find(players_white.begin(), players_white.end(), players[whiteID]));
		players_black.erase(find(players_black.begin(), players_black.end(), players[blackID]));
		
		auto pos = find(players_white.begin(), players_white.end(), players[blackID]);
		if(pos != players_white.end())
		    players_white.erase(pos);

		pos = find(players_black.begin(), players_black.end(), players[whiteID]);
		if(pos != players_black.end())
		    players_black.erase(pos);


		for (const auto &p : players_white) {
		    fmt::print("{}, ", p.playerID);
		}
		fmt::print("\n");

		for (const auto &p : players_black) {
		    fmt::print("{}, ", p.playerID);
		}
		fmt::print("\n");

		return true;
	    }
	}

	// Aqui te puedes printear los jugadores disponibles de blancas y de negras...
	// fmt::print("Failed to make pairing\n");
	return false;
    }

    bool is_new_game(const Game &game) const { return std::find(games.begin(), games.end(), game) == games.end(); }

    void create_matches_round(std::vector<Player> &players_white, std::vector<Player> &players_black, uint64_t day) {
	static const uint32_t MAX_TRIES = 10;
	assert(players_white.size() < MAX_TRIES);
	assert(players_black.size() < MAX_TRIES);

	uint64_t matchCounter = 0;

	// In case it doesnt work, to avoid infinite loops
	for (uint32_t i = 0; i < MAX_TRIES; i++) {
	    if (create_match(players_white, players_black, day)) {
		matchCounter++;
	    }

	    // Every matchup created
	    if (matchCounter == (num_players - 1) / 2) {
		player_id restID = -1;
		if (players_white.size()) {
		    restID = players_white.front().playerID;
		    fmt::print("Black rests {}!\n", restID);
		}

		if (players_black.size()) {
		    restID = players_black.front().playerID;
		    fmt::print("White rests {}!\n", restID);
		}
        if(restID != -1)//hacia segfault porque en los vectores de negro/blanco no queda ninguno entonces lo hacia con el valor sin restID sin inicializar
		    players[restID].hasRested = true;

		fmt::print("Round done!\n");
		break;
	    }
	}
    }

    // void create_matchups() {
    // const uint64_t days = num_players;
    //// we assign the ID to the players
    // for (uint64_t player = 0; player < num_players; player++) {
    // players[player].playerID = player;
    // players[player].black.resize(num_players);
    // players[player].white.resize(num_players);
    //// players[player].played.resize(num_players);
    //}

    // for (uint64_t day = 0; day < days; day++) {
    // fmt::print("DAY {}\n", day);

    //// sort by ascending points the players each day
    // fmt::print("Ordered vector: ");
    // std::sort(players.begin(), players.end(),
    //[day](Player &x, Player &y) { return x.points_per_day[day] < y.points_per_day[day]; });
    // for (int i = 0; i < num_players; i++) {
    // fmt::print("{} ", players[i].playerID);
    //}
    // fmt::print("\n");
    // uint64_t matchCounter = 0;
    // for (uint64_t white = 0; white < num_players; white++) {

    // for (uint64_t black = 0; black < num_players;
    // black++) { // TODO ver que si blanco o negro ha jugado hoy no puede volver a jugar

    // if (matchCounter == (num_players - 1) / 2) { // comprueba partidas del dia
    // fmt::print("{} matched plays\n\n", matchCounter);
    // break;
    //}

    // fmt::print("{} blanco - {} negro\n", players[white].playerID, players[black].playerID);
    // fmt::print("\nLISTA DE BLANCOS DEL BLANCO: ");

    // fmt::print("{} ", players[white].white[day]);

    // fmt::print("\n BLANCO DE NEGRO: {}", players[white].black[day]);
    // fmt::print("\nLISTA DE LOS NEGROS de negro: ");

    // fmt::print("{} ", players[black].black[day]);

    // fmt::print("\n NEGRO DE BLANCO: {}", players[black].white[day]);

    // if (white != black) { // you cannot play vs yourself
    // bool repeated = false;
    //// comprobamos que ninguno de los jugadores haya jugado hoy
    // if (players[white].white[day] || players[white].black[day]) {
    // fmt::print("\n\nBLANCO {} ha jugado ya hoy\n\n", players[white].playerID);
    // repeated = true;
    // break;
    //} else if (players[black].black[day] || players[black].white[day]) {
    // fmt::print("\n\nNEGRO {} ha jugado ya hoy\n\n", players[black].playerID);
    // repeated = true;
    //// continue; nos ahorramos todo el codigo de dentro
    //}

    //// comprobamos que lleve la mitad de partidas como blanca/negra

    //[>if(day != 0 && (players[white].whiteMatches >= (days + 1)/ 2 || players[black].blackMatches >=
    //(days + 1) / 2)){ break;
    //}*/
    //[>int64_t diffW = players[white].whiteMatches - players[white].blackMatches;
    // int64_t diffB = players[black].blackMatches - players[black].blackMatches;
    // if(diffW > 0){
    // fmt::print("Blanco ha jugado demasiado como blanco {}\n", diffW);
    // continue;
    //}

    // if(diffB > 0){
    // fmt::print("Negro ha jugado demasiado como negro {}\n", diffB);
    // continue;
    //}*/

    // const Match m{day, players[white].playerID, players[black].playerID};

    //// we look if that match have been done previously

    // fmt::print("Looking for {} - {}\n", players[white].playerID, players[black].playerID);
    // const Game prevMatch{players[white].playerID, players[black].playerID};
    // const Game prevMatchRev{players[black].playerID, players[white].playerID};

    //// const auto matches_in_day = matches.equal_range(prevMatch);

    //// comparo con los partidos blanco-negro y partidos del reves (blanco era negro y viceversa)
    // auto pos = std::find(games.begin(), games.end(), prevMatch);
    // auto posRev = std::find(games.begin(), games.end(), prevMatchRev);

    // if (pos != games.end()) { // miro que no se repita con el normal
    // fmt::print("Match repetido saltamos\n");
    // repeated = true;

    //} else if (posRev != games.end()) { // miro que no se repita con el inverso
    // fmt::print("Match Repetido a la inversa saltamos\n");
    // repeated = true;
    //}

    // if (!repeated) { // si el match no es repetido lo inserta
    // fmt::print("Insert\n");
    // games.push_back(Game{players[white].playerID, players[black].playerID});
    // matches.insert(m);
    // players[white].white[day] = true;
    // players[white].black[day] = false;
    // players[white].whiteMatches++;
    // players[black].black[day] = true;
    // players[black].white[day] = false;
    // players[black].blackMatches++;
    // matchCounter++; // counter para parar las partidas del dia X en caso de que sobre ya haya
    //// los partidos
    // break;
    //}

    //} else
    // fmt::print("NO POSSIBLE VS YOURSELF\n");
    //}
    // if (matchCounter == (num_players - 1) / 2) { // comprueba partidas del dia
    // fmt::print("{} matched plays\n\n", matchCounter);

    // break;
    //}
    //}
    //}

    /*
     *   Players can't play vs themselves
     *   Every player has to play with every other player
     *   etc
     *
     *   For each day D sort players (increasingly) by the points
     *   they win by resting this day, and create matchups from this,
     *   taking into account impossible matchups and so on
     */
    //}