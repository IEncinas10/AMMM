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

enum class Color { WHITE, BLACK };

struct Player {

    player_id playerID;
    std::vector<uint64_t> points_per_day;
    uint64_t games_black = 0;
    uint64_t games_white = 0;
    bool hasRested = false;

    std::size_t &operator[](std::size_t index) { return points_per_day[index]; }
    std::size_t operator[](std::size_t index) const { return points_per_day[index]; }

    static bool can_play_white(const Player &p) { return p.games_white < (p.total_games() + 1) / 2; }
    static bool can_play_black(const Player &p) { return p.games_black < (p.total_games() + 1) / 2; }

    bool can_play_white() const { return games_white < (total_games() + 1) / 2; }
    bool can_play_black() const { return games_black < (total_games() + 1) / 2; }

    bool operator==(const Player &other) const { return playerID == other.playerID; }

    bool has_missing_games() const { return points_per_day.size() - 1 != games_black + games_white; }
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
	static const uint32_t MAX_TRIES = 10;
	const uint64_t days = num_players;
	assign_player_ids();
	fmt::print("Creating all combination matches....\n"); // we just create the possible matches we have, we will
							      // need to evaluate the points in each day.
	// TODO: assign colors from this point forward
	create_games();

	fmt::print("All matches possible matches created in C.\n");

	for (uint64_t day = 0; day < days; day++) {
	    uint64_t todayMatches = 0, tries = 0;

	    std::vector<uint64_t> todayPlayer;
	    fmt::print("DAY {}\nOrdering C by points\n", day);

	    std::sort(games.begin(), games.end(),
		      [&](const Game &x, const Game &y) { // sorting by less points per match
			  return players.at(x.white).points_per_day[day] + players.at(x.black).points_per_day[day] <
				 players.at(y.white).points_per_day[day] + players.at(y.black).points_per_day[day];
		      });

	    fmt::print("\n[{}] ", games.size());
	    for (Game &g : games) {
		fmt::print("{} - {}, ", g.white, g.black);
	    }
	    fmt::print("\n");

	    while (todayMatches < (num_players - 1) / 2) { // restriccion de partidos por dia (?)
		if (find_best_match(games, todayPlayer, day)) {
		    todayMatches++;
		} else {
		    fmt::print("no encontrado {}\n", tries);
		    tries++;
		    if (tries > MAX_TRIES) {
			break;
		    }
		}
	    }

	    mark_has_rested(todayPlayer);

	    if (tries > MAX_TRIES) {
		fmt::print("FFFFFFFFFFFF\n");
	    } else
		fmt::print("DAY {} COMPLETED\n", day);
	}
    }

    bool player_can_play(player_id p, const std::vector<player_id> &playedToday) {
	return std::find(playedToday.begin(), playedToday.end(), p) == playedToday.end();
    }

    void mark_has_rested(const std::vector<player_id> &playedToday) {
	for (player_id i = 0; i < num_players; i++) {
	    // If a played hasn't play today it is going to rest
	    if (player_can_play(i, playedToday)) {
		players[i].hasRested = true;
	    }
	}
    }

    bool find_best_match(std::vector<Game> &games, std::vector<uint64_t> &todayPlayer, uint64_t day) {
	// we search in array games (C) the feasibles matchups and the best suitable
	
	for(uint64_t i = 0; i < games.size(); i++) {
	    Game &g = games[i];
	    uint64_t white = g.white;
	    uint64_t black = g.black;
	    // fmt::print("LOOKING FOR MATCH\n");

	    // we check that both players didnt play today
	    if (!player_can_play(white, todayPlayer) || !player_can_play(black, todayPlayer))
		continue;

	    const Match m{day, white, black};

	    // TODO:
	    //
	    // https://github.com/IEncinas10/AMMM/issues/1
	    // "Asegurar solucion"
	    //



	    //
	    //
	    //

	    fmt::print("Inserting match {} - {} day {}.\n", white, black, day);
	    matches.insert(m);

	    //Remove, dont care. We should check this when we create matches before doing the algorithm
	    players[white].games_white++;
	    players[black].games_black++;
	    //


	    todayPlayer.push_back(white);
	    todayPlayer.push_back(black);
	    games.erase(games.begin() + i);

	    return true;
	}

	return false;
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
		fmt::print("Player {} has played {} white games and {} black games\n", p.playerID, p.games_white,
			   p.games_black);
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

void print_usage(const char *program_name) { fmt::print("Usage: {} instance_filepath\n", program_name); }

int main(int argc, char **argv) {
    if (argc < 2) {
	print_usage(argv[0]);
    }

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
