#include <boost/algorithm/string.hpp>
#include <cstdint>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

using player_id = uint64_t;

float alpha = 0;

struct Player {

    player_id playerID;
    std::vector<uint64_t> points_per_day;
    uint64_t games_played = 0;
    bool hasRested = false;

    std::size_t &operator[](std::size_t index) { return points_per_day[index]; }
    std::size_t operator[](std::size_t index) const { return points_per_day[index]; }

    bool operator==(const Player &other) const { return playerID == other.playerID; }

    bool has_missing_games() const { return points_per_day.size() - 1 != games_played; }
    uint64_t total_games() const { return points_per_day.size() - 1; }
};

struct Match {
    uint64_t day;
    player_id white;
    player_id black;

    bool operator<(const Match &other) const { return day < other.day; }

    bool operator==(const Match &other) const {
	return day == other.day && white == other.white && black == other.black;
    }
};

struct Game {
    player_id white;
    player_id black;

    bool operator==(const Game &other) const {
	return (white == other.white && black == other.black) || (white == other.black && black == other.white);
    }

    bool operator!=(const Game &other) const { return !((*this) == other); }
};

struct Tournament {
    std::size_t num_players;
    // We can get the number of days from the
    // number of players
    // std::size_t num_days = num_players;

    std::vector<Player> players;
    std::multiset<Match> matches;
    std::vector<Game> gamesPlayed;
    std::vector<Game> games;

    void set_num_players(std::size_t num_players_) {
	num_players = num_players_;
	players.resize(num_players);
    }

    void assign_player_ids() {
	for (uint64_t player = 0; player < num_players; player++) {
	    players[player].playerID = player;
	}
    }

    void create_games() {
	for (uint64_t x = 0; x < num_players; x++) {
	    for (uint64_t y = x + 1; y < num_players; y++) {
		if (x == y) // we dont add matches vs yourself
		    continue;

		uint64_t white, black;
		if ((x + y) % 2 == 0) {
		    white = x;
		    black = y;
		} else {
		    white = y;
		    black = x;
		}

		Game g{white, black};
		games.push_back(g); // GAMES IS NOW THE CANDIDATE VECTOR
	    }
	}

	// create a function to check that every player has the same games as black thn as white
	fmt::print("\n[{}] ", games.size());
	for (Game &g : games) {
	    fmt::print("{} - {}, ", g.white, g.black);
	}
	fmt::print("\n");
    }

    void create_matchups() {

	// we assign the ID to the players
	const uint64_t days = num_players;
	std::vector<uint64_t> rests; // vector with ID of player that rests every day
	uint64_t score = 0;

	assign_player_ids();
	fmt::print("Creating all combination matches....\n"); // we just create the possible matches we have, we will
							      // need to evaluate the points in each day.
	// TODO: assign colors from this point forward
	create_games();

	fmt::print("All matches possible matches created in C.\n");

	for (uint64_t day = 0; day < days; day++) {
	    // Make a copy to sort it by points this round
	    std::vector<Player> players_round(players);

	    fmt::print("DAY {}\n Ordering C by points\n", day);

	    std::sort(players_round.begin(), players_round.end(),
		      [&](const Player &x, Player &y) { // sorting by less points per match
			  return x.points_per_day[day] > y.points_per_day[day];
		      });

	    assign_rest(players_round, day, rests, score);
	}

	auto points = 0;
	fmt::print("SOLUTION GREEDY:\n[");
	for (uint32_t i = 0; i < rests.size(); i++) {
	    fmt::print("{} ", rests[i]);
	    points += players[rests[i]].points_per_day[i];
	}

	fmt::print("]\nFinal Score: {}\n", points);

	local_search(rests);

	make_calendar(days, rests);
    }

    void omfg(const std::vector<player_id> &rests) {
	// Idea: fake a tournament with 2k players instead of 2k -1
	// The "last" player is actually the rest day
	// then map playsfake(day d) with rests[day d]
	//
	std::vector<player_id> fakerest;
	for (uint32_t day = 0; day < num_players; day++) {
	    //fmt::print("\n\nDay {}\n", day);
	    for (player_id p = 0; p <= num_players / 2; p++) {
		// actually choose color correctly with parity blabla
		player_id w = (p + day) % num_players;
		player_id b = (day + num_players - 1 - p) % num_players;
		//fmt::print("{} - {}\n", w, b);
		if (w == b)
		    fakerest.push_back(w);
	    }
	}

	fmt::print("Fake rest: [{}]\n", fmt::join(fakerest, " "));
	fmt::print("Rest:      [{}]\n", fmt::join(rests, " "));

	for (uint32_t day = 0; day < num_players; day++) {
	    for (player_id p = 0; p <= num_players / 2; p++) {
		// actually choose color correctly with parity blabla
		player_id w = (p + day) % num_players;
		player_id b = (day + num_players - 1 - p) % num_players;


		uint32_t rename_index_w = std::find(fakerest.begin(), fakerest.end(), w) - fakerest.begin();
		uint32_t rename_index_b = std::find(fakerest.begin(), fakerest.end(), b) - fakerest.begin();
		if (b > w && (w + b) % 2 != 0) {
		    std::swap(w, b);
		}

		w = rests[rename_index_w];
		b = rests[rename_index_b];


		if (w == rests[day]) {
		    assert(w == b);
		    continue;
		}


		assert(w != b);
		players[w].games_played++;
		players[b].games_played++;
		const Match m{day, w, b};
		matches.insert(m);
	    }
	}
    }

    void make_calendar(const uint64_t days, std::vector<player_id> &rests) {

	omfg(rests);
	return;

	for (uint64_t day = 0; day < days; day++) {

	    uint64_t todayMatches = 0;
	    uint64_t tries = 0;
	    bool succeed = create_matches_day(games, day, rests[day]);
	    if (!succeed) {
		fmt::print("Couldn't create every match for day {}\n", day);
	    }
	}
    }

    std::vector<player_id> remainingAdvs(player_id id) {
	std::vector<player_id> remainingGames;
	// buscar si existen los partidos con id

	for (const Game &g : games) {
	    if (g.white == id)
		remainingGames.push_back(g.black);
	    else if (g.black == id)
		remainingGames.push_back(g.white);
	}
	return remainingGames;
    }

    bool create_matches_day(std::vector<Game> &games, uint64_t day, player_id rests_today) {
	// we search in array games (C) the feasibles matchups and the best suitable

	std::set<player_id> played_today;

	fmt::print("\n\nDay {}. Rests {}\n", day, rests_today);
	std::vector<Player> players_sort_by_nummatches(players);
	std::sort(players_sort_by_nummatches.begin(), players_sort_by_nummatches.end(),
		  [](const Player &x, const Player &y) { return x.games_played > y.games_played; });

	for (uint64_t p = 0; p < players_sort_by_nummatches.size(); p++) {
	    if (players_sort_by_nummatches[p].playerID == rests_today ||
		played_today.count(players_sort_by_nummatches[p].playerID))
		continue;

	    fmt::print("\nTrying to find match for {}\n==========\n", players_sort_by_nummatches[p].playerID);

	    for (uint64_t i = 0; i < games.size(); i++) {
		Game &g = games[i];
		uint64_t white = g.white;
		uint64_t black = g.black;
		if (white != players_sort_by_nummatches[p].playerID &&
		    black != players_sort_by_nummatches[p].playerID) {
		    continue;
		}

		const Match m{day, white, black};
		fmt::print("Trying match {} - {} day {}.\n", white, black, day);

		// Check match is valid, no player is resting
		if (white == rests_today || black == rests_today) {
		    fmt::print("Cant. Some player is resting\n");
		    continue;
		}

		// Check player hasn't played today
		if (played_today.count(white) || played_today.count(black)) {
		    fmt::print("Some player has played\n");
		    continue;
		}

		played_today.insert(white);
		played_today.insert(black);

		fmt::print("Inserting match {} - {} day {}.\n", white, black, day);
		matches.insert(m);

		// Remove, dont care. We should check this when we create matches before doing the algorithm
		players[white].games_played++;
		players[black].games_played++;
		//

		games.erase(games.begin() + i);
		// we have erased one game
		i--;

		break;
		if (played_today.size() == num_players - 1)
		    goto xd;
	    }

	    if (!played_today.count(players_sort_by_nummatches[p].playerID)) {
		fmt::print("Couldn't find match for {}\n", players_sort_by_nummatches[p].playerID);
	    }
	}

    xd:
	fmt::print("\nPlayers: [");
	for (player_id i = 0; i < num_players; i++) {
	    if (played_today.count(i))
		fmt::print("{} ", i);
	}
	fmt::print("]\n");
	return played_today.size() == num_players - 1;
    }

    void local_search(std::vector<player_id> &rests) {
	uint64_t num_days = num_players;

	for (uint32_t i = 0; i < num_days; i++) {
	    int best_swap = i;
	    int best_swap_points = 0;
	    for (uint32_t j = 0; j < num_days; j++) {
		auto curr_points = players[rests[i]].points_per_day[i] + players[rests[j]].points_per_day[j];
		auto swap_points = players[rests[j]].points_per_day[i] + players[rests[i]].points_per_day[j];

		int change = swap_points - curr_points;
		if (change > best_swap_points) {
		    fmt::print("Swapping {} and {}. Change: {}. Prev: {}\n", rests[i], rests[j], change,
			       best_swap_points);
		    best_swap = j;
		    best_swap_points = change;
		}
	    }

	    std::swap(rests[i], rests[best_swap]);
	}

	uint32_t points = 0;
	fmt::print("LOCAL SEARCH: [");
	for (uint32_t i = 0; i < num_players; i++) {
	    fmt::print("{} ", rests[i]);
	    points += players[rests[i]].points_per_day[i];
	}

	fmt::print("] Points: {}\n", points);

	return;
    }

    void assign_rest(std::vector<Player> &players_day, uint64_t day, std::vector<player_id> &rests, uint64_t &score) {

	std::srand(std::time(nullptr)); // use current time as seed for random generator
	// Para que GRASP vaya hay que quitarse los jugadores que han descansado de "players_day", si no
	// no va a ir
	std::vector<Player> clean_players;
	uint64_t chosen_index = 0;

	std::copy_if(players_day.begin(), players_day.end(), back_inserter(clean_players),
		     [](Player &x) { return !x.hasRested; });

	uint64_t last_index = (clean_players.size() - 1) * alpha;
	if (last_index != 0)
	    chosen_index = std::rand() % last_index;

	Player player = clean_players[chosen_index];
	score += player.points_per_day[day];
	rests.push_back(player.playerID);
	players[player.playerID].hasRested = true;
	fmt::print("Player {} rests in day {} with {} points\n", player.playerID, day, player.points_per_day[day]);
    }

    bool player_can_play(player_id p, const std::vector<player_id> &playedToday) {
	return std::find(playedToday.begin(), playedToday.end(), p) == playedToday.end();
    }

    bool contains(const std::vector<player_id> &adv, player_id player) {
	return std::find(adv.begin(), adv.end(), player) != adv.end();
    }

    void print() {
	fmt::print("{} players\n", num_players);
	for (const auto &p : players) {
	    fmt::print("[{}]\n", fmt::join(p.points_per_day, ", "));
	}

	create_matchups();
	fmt::print("[\n\t{}\n]\n", fmt::join(matches, "\n\t"));

	for (const auto &p : players) {
	    if (p.has_missing_games())
		fmt::print("Player {} has played {} games\n", p.playerID, p.games_played);
	}
    }
};

void get_num_players(const std::string &current_line, uint64_t &number_of_players, uint64_t &remaining_players) {
    std::istringstream stream(current_line);
    if (current_line.find('n') != std::string::npos) {
	char dummy;
	stream >> dummy >> dummy >> number_of_players;
	remaining_players = number_of_players;
    }
}

bool read_instance(const char *instance_filename, Tournament &tournament) {
    std::ifstream input(instance_filename);
    if (!input.is_open())
	return false;

    uint64_t lines_read = 0, number_of_players = 0, remaining_players = UINT64_MAX, player_index = 0;
    bool encountered_p = false;
    std::string current_line;
    do {
	std::getline(input, current_line);
	boost::algorithm::trim_if(current_line, boost::algorithm::is_any_of("[,], ,\t"));

	// We haven't read players yet
	if (number_of_players == 0) {
	    get_num_players(current_line, number_of_players, remaining_players);
	    tournament.set_num_players(number_of_players);
	} else if (!encountered_p) {
	    encountered_p = current_line.find('p') != std::string::npos;
	} else if (!current_line.empty()) {
	    std::istringstream stream(current_line);

	    uint64_t number_of_days = number_of_players, tmp = 0;
	    for (uint64_t j = 0; j < number_of_days; j++) {
		stream >> tmp;
		tournament.players[player_index].points_per_day.push_back(tmp);
	    }
	    player_index++;
	    remaining_players--;
	}

	lines_read++;
    } while (!input.eof() && remaining_players != 0);

    fmt::print("Read {} lines\n", lines_read);

    tournament.print();
    return true;
}

void print_usage(const char *program_name) { fmt::print("Usage: {} instance_filepath alpha\n", program_name); }

void get_alpha(int argc, char **argv) {
    if (argc >= 3) {
	alpha = std::stof(argv[2]);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
	print_usage(argv[0]);
    }
    get_alpha(argc, argv);

    const char *instance_filename = argv[1];

    Tournament tournament;
    read_instance(instance_filename, tournament);
}

template <> struct fmt::formatter<Match> {
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
	// [ctx.begin(), ctx.end()) is a character range that contains a part of
	// the format string starting from the format specifications to be parsed,
	// e.g. in
	//
	//   fmt::format("{:f} - point of interest", point{1, 2});
	//
	// the range will contain "f} - point of interest". The formatter should
	// parse specifiers until '}' or the end of the range. In this example
	// the formatter should parse the 'f' specifier and return an iterator
	// pointing to '}'.

	// Please also note that this character range may be empty, in case of
	// the "{}" format string, so therefore you should check ctx.begin()
	// for equality with ctx.end().

	// Parse the presentation format and store it in the formatter:
	auto it = ctx.begin(), end = ctx.end();
	if (it != end && (*it == 'f' || *it == 'e'))
	    presentation = *it++;

	// Check if reached the end of the range:
	if (it != end && *it != '}')
	    throw format_error("invalid format");

	// Return an iterator past the end of the parsed range:
	return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext> auto format(const Match &m, FormatContext &ctx) const -> decltype(ctx.out()) {
	// ctx.out() is an output iterator to write to.
	return fmt::format_to(ctx.out(), "[Day: {}, White: {}, Black: {}]", m.day, m.white, m.black);
    }
};
