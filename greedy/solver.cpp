#include "../include/cxxopts.hpp"
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>

using player_id = uint64_t;

static std::string instance_filename_str;
static std::mt19937 generator;

// Alpha for GRASP. Default to 0, read from command line arguments
float alpha = 0;

// Algorithm selected [0, greedy] [1, localsearch]

bool useLocalSearch = false;

// whether to print or not the full calendar
bool print_calendar = true;

// hack to print nicely the tournament schedule
std::size_t NUM_PLAYERS;

struct Player {
    // Player input
    std::vector<uint64_t> points;

    // ID, just for bookkeeping
    player_id playerID;

    // Check whether or not a player has rested. Needed to avoid leting a player rest more than once
    bool has_rested = false;

    // To do checks after getting a tournament, it's not really needed
    uint64_t games_played = 0, games_white = 0, games_black = 0;

    bool operator==(const Player &other) const { return playerID == other.playerID; }

    bool has_missing_games() const { return points.size() - 1 != games_played; }
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

// This struct is not too useful, we just use to check our solution
struct Game {
    player_id white;
    player_id black;
};

struct Tournament {
    std::size_t num_players;
    // We can get the number of days from the
    // number of players
    // std::size_t num_days = num_players;

    std::vector<Player> players;

    // Tournament schedule: {Day, player1, player2}
    std::multiset<Match> matches;

    // Just for checking solution
    std::vector<Game> games;

    void set_num_players(std::size_t num_players_) {
	num_players = num_players_;
	players.resize(num_players);
    }

    void assign_player_ids() {
	for (uint64_t player = 0; player < num_players; player++) {
	    players.at(player).playerID = player;
	    // players[player].playerID = player;
	}
    }

    void clear_players_attributes() {
	for (uint64_t player = 0; player < num_players; player++) {
	    players.at(player).has_rested = false;
	    players.at(player).games_played = 0;
	    players.at(player).games_black = 0;
	    players.at(player).games_white = 0;
	}
    }

    // We create a set of games where every player plays against
    // every other player (50% white, 50% black)
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
		games.push_back(g);
	    }
	}
    }

    void create_matchups() {

	// vector with ID of player that rests every day

	std::vector<uint64_t> bestRests;
	const uint64_t days = num_players;
	uint64_t nIter = 0;
	uint64_t MAXGRASP = 100;
	uint64_t bestScore = 0;
	uint64_t notImproved = 0;
	uint64_t NOT_IMPROVED_MAX = 10;

	// we assign the ID to the players
	assign_player_ids();

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	do {
	    std::vector<uint64_t> rests;
	    uint64_t score = 0;
	    nIter++;
	    for (uint64_t day = 0; day < days; day++) {
		// Make a copy to sort it by points this round
		std::vector<Player> players_round(players);

		// fmt::print("DAY {}\n Ordering C by points\n", day);

		std::sort(players_round.begin(), players_round.end(),
			  [&](const Player &x, Player &y) { // sorting by less points per match
			      return x.points[day] > y.points[day];
			  });

		assign_rest(players_round, day, rests, score);
	    }

	    //fmt::print("SOLUTION GREEDY: [{}]\nPoints: {}\n", fmt::join(rests, " "), score);

	    if (useLocalSearch == true)
		local_search(rests, score);

	    notImproved++;
	    if (score >= bestScore) {
		if (score > bestScore)
		    notImproved = 0;
		bestScore = score;
		bestRests = rests;
	    }

	    clear_players_attributes();

	} while (notImproved < NOT_IMPROVED_MAX && nIter < MAXGRASP && alpha != 0);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Rest vector time (s): "
		  << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1000000000.0 << '\n';

	fmt::print("Final score and solution [{}]\nPoints: {}\n", fmt::join(bestRests, " "), bestScore);

	// Generate set of valid games (just for validating solution, this does nothing right else now)
	create_games();
	make_calendar(days, bestRests);
    }

    void make_calendar(uint64_t days, const std::vector<player_id> &rests) {
	// Idea: fake a tournament with 2k players instead of 2k -1
	// The "last" player is actually the rest day
	// then map playsfake(day d) with rests[day d]

	// In reality: we do a 2k-1 players tournament but with self match
	// then, we use this "self match" as a rest day, and obtain
	// a fake rest day. After that, we rename the players like this:
	//
	// fakerest[day] <- rest[day]
	//
	// Then, every player rests in his desired day

	std::vector<player_id> fakerest;
	for (uint32_t day = 0; day < days; day++) {
	    // fmt::print("\n\nDay {}\n", day);
	    for (player_id p = 0; p <= num_players / 2; p++) {
		// actually choose color correctly with parity blabla
		player_id w = (p + day) % num_players;
		player_id b = (day + num_players - 1 - p) % num_players;
		// fmt::print("{} - {}\n", w, b);
		if (w == b)
		    fakerest.push_back(w);
	    }
	}

	uint8_t id_width = std::log10(num_players) + 1;
	fmt::print("Fake rest: [{:{}}]\nRest:      [{:{}}]\n", fmt::join(fakerest, " "), id_width,
		   fmt::join(rests, " "), id_width);

	for (uint32_t day = 0; day < days; day++) {
	    for (player_id p = 0; p <= num_players / 2; p++) {
		player_id w = (p + day) % num_players;
		player_id b = (day + num_players - 1 - p) % num_players;

		uint32_t rename_index_w = std::find(fakerest.begin(), fakerest.end(), w) - fakerest.begin();
		uint32_t rename_index_b = std::find(fakerest.begin(), fakerest.end(), b) - fakerest.begin();

		// Color fairness
		if (b > w && (w + b) % 2 != 0) {
		    std::swap(w, b);
		}

		w = rests[rename_index_w];
		b = rests[rename_index_b];

		if (w == rests[day]) {
		    assert(w == b);
		    continue;
		}

		// Can't play vs yourself
		assert(w != b);

		players[w].games_played++;
		players[w].games_white++;
		players[b].games_played++;
		players[b].games_black++;
		const Match m{day, w, b};
		matches.insert(m);
	    }
	}

	// Check that every game is present in our "matches" structure
	for (const Game &g : games) {
	    bool found = false;
	    for (uint32_t day = 0; day < num_players; day++) {
		const Match m{day, g.white, g.black};
		if (matches.count(m))
		    found = true;
	    }

	    assert(found);
	}
	assert(games.size() == matches.size());

	// Additionally, check that every player plays a correct amount of games and they're 50%w, 50%b
	for (const Player &p : players) {
	    assert(!p.has_missing_games());
	    assert(p.games_white == (num_players - 1) / 2);
	    assert(p.games_black == (num_players - 1) / 2);
	}
    }

    void local_search(std::vector<player_id> &rests, uint64_t &points) {
	uint64_t num_days = num_players;
	uint64_t prev_solution = points;
	uint64_t old_points;

	uint64_t i = 0;

	do {
	    old_points = points;

	    for (uint32_t day = 0; day < num_days; day++) {
		int best_swap = day;
		int best_swap_points = 0;
		for (uint32_t j = 0; j < num_days; j++) {
		    auto curr_points = players[rests[day]].points[day] + players[rests[j]].points[j];
		    auto swap_points = players[rests[j]].points[day] + players[rests[day]].points[j];

		    int change = swap_points - curr_points;
		    if (change > best_swap_points) {
			//fmt::print("Swapping {} and {}. Change: {}. Prev: {}\n", rests[day], rests[j], change,
				   //best_swap_points);
			best_swap = j;
			best_swap_points = change;
		    }
		}

		std::swap(rests[day], rests[best_swap]);
		points += best_swap_points;
	    }
	    i++;
	} while (points > old_points);

	//fmt::print("Points after local search: {}\n", points);
	//fmt::print("Improvement by LS in {} iterations: {}\n", i, points - prev_solution);
    }

    void assign_rest(std::vector<Player> &players_day, uint64_t day, std::vector<player_id> &rests, uint64_t &score) {

	// use current time as seed for random generator

	// Para que GRASP vaya hay que quitarse los jugadores que han descansado de "players_day"
	std::vector<Player> clean_players;
	std::vector<Player> RCL_players;

	std::copy_if(players_day.begin(), players_day.end(), back_inserter(clean_players),
		     [](Player &x) { return !x.has_rested; });

	uint64_t qmin = clean_players[clean_players.size() - 1].points[day];
	uint64_t qmax = clean_players[0].points[day];

	uint64_t worst_possible_points = qmax - alpha * (qmax - qmin);

	std::copy_if(clean_players.begin(), clean_players.end(), back_inserter(RCL_players),
		     [&](Player &x) { return x.points[day] >= worst_possible_points; });

	// GRASP. If alpha 0 defaults to normal greedy
	uint64_t chosen_index = 0, last_index = RCL_players.size();
	
	if (last_index != 0) {
	    std::uniform_int_distribution<uint32_t> dist(0, last_index - 1);
	    chosen_index = dist(generator);
	}

	const Player &player = RCL_players[chosen_index];
	score += player.points[day];
	rests.push_back(player.playerID);
	players[player.playerID].has_rested = true;

	// fmt::print("Player {} rests in day {} with {} points\n", player.playerID, day, player.points_per_day[day]);
    }

    void print() {
	fmt::print("{} players\n", num_players);
	for (const auto &p : players) {
	    fmt::print("[{:3}]\n", fmt::join(p.points, ", "));
	}

	fmt::print("[\n\t{}\n]\n", fmt::join(matches, "\n\t"));

	for (const auto &p : players) {
	    if (p.has_missing_games())
		fmt::print("Player {} has played {} games. {} w, {} b\n", p.playerID, p.games_played, p.games_white,
			   p.games_black);
	}
    }
};

/***************************************************/
/*************** INSTANCE READING ******************/
/***************************************************/

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
	    NUM_PLAYERS = number_of_players;
	} else if (!encountered_p) {
	    encountered_p = current_line.find('p') != std::string::npos;
	} else if (!current_line.empty()) {
	    std::istringstream stream(current_line);

	    uint64_t number_of_days = number_of_players, tmp = 0;
	    for (uint64_t j = 0; j < number_of_days; j++) {
		stream >> tmp;
		tournament.players[player_index].points.push_back(tmp);
	    }
	    player_index++;
	    remaining_players--;
	}

	lines_read++;
    } while (!input.eof() && remaining_players != 0);

    fmt::print("Read {} lines\n", lines_read);

    return true;
}

/***************************************************/
/***************************************************/
/***************************************************/

void print_usage(const char *program_name) { fmt::print("Usage: {} instance_filepath alpha\n", program_name); }

void parse(int argc, char **argv) {
    try {
	cxxopts::Options options(argv[0], "AMMM Course Project 2022\nSolver");

	// cxxopts::value<uint32_t>()->default_value("1"))

	// clang-format off
	options.set_width(90).set_tab_expansion().add_options()
	    ("i, instance", "Path to the instance to solve", cxxopts::value<std::string>())
	    ("l, localsearch", "Enable local search. It's implied whenever we do GRASP")
	    ("no-calendar", "Avoid printing full calendar and point matrix")
	    ("a, alpha", "Set alpha for GRASP [0-1]. If alpha is different to (0) GRASP algorithm will be set", cxxopts::value<float>()->default_value("0"))
	    ("h, help", "Print help");
	// clang-format on

	auto result = options.parse(argc, argv);

	if (result.count("help")) {
	    std::cout << options.help() << std::endl;
	    exit(0);
	}

	instance_filename_str = result["instance"].as<std::string>();
	useLocalSearch = result.count("localsearch");
	alpha = result["alpha"].as<float>();

	print_calendar = !result.count("no-calendar");

	if (alpha != 0) {
	    useLocalSearch = true;
	}

    } catch (const cxxopts::exceptions::exception &e) {
	std::cout << "error parsing options: " << e.what() << std::endl;
	exit(-1);
    }
}

int main(int argc, char **argv) {
	generator.seed(0);

    parse(argc, argv);
    if (argc < 2) {
	print_usage(argv[0]);
    }

    fmt::print("Alpha: {}\n", alpha);

    const char *instance_filename = instance_filename_str.c_str();

    Tournament tournament;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    bool read_ok = read_instance(instance_filename, tournament);
    assert(read_ok);
    tournament.create_matchups();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time (s): "
	      << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1000000000.0 << std::endl;

    if(print_calendar)
	tournament.print();
}

// fmtlib stuff, copypasted from somewhere and modified to fit our Match struct
template <> struct fmt::formatter<Match> {
    char presentation = 'f';

    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
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
	// Print matches width-aligned to make it nicer
	uint8_t id_width = std::log10(NUM_PLAYERS) + 1;
	return fmt::format_to(ctx.out(), "[Day: {0:{3}}, White: {1:{3}}, Black: {2:{3}}]", m.day, m.white, m.black,
			      id_width);
    }
};
