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

    void create_matchups() {
	// we assign the ID to the players
	static const uint32_t MAX_TRIES = 100;
	const uint64_t days = num_players;
	for (uint64_t player = 0; player < num_players; player++) {
	    players[player].playerID = player;
	}

	fmt::print("Creating all combination matches....\n"); // we just create the possible matches we have, we will
							      // need to evaluate the points in each day.

	for (uint64_t white = 0; white < num_players; white++) {
	    for (uint64_t black = 0; black < num_players; black++) {
		if (white == black) // we dont add matches vs yourself
		    continue;
		Game g{players[white].playerID, players[black].playerID};
		games.push_back(g); // GAMES IS NOW THE CANDIDATE VECTOR
	    }
	}

	fmt::print("All matches possible matches created in C.\n");

	for (u_int64_t day = 0; day < days; day++) {
	    uint64_t todayMatches = 0;
	    static const uint32_t MAX_TRIES = 10;
	    uint32_t tries = 0;
	    std::vector<uint64_t> todayPlayer;
	    fmt::print("DAY {}\nOrdering C by points\n", day);

	    const std::vector<Player> players_day(players);

	    std::sort(games.begin(), games.end(),
		      [day, players_day](const Game &x, const Game &y) { // sorting by less points per match
			  if ((players_day[x.white].hasRested || players_day[x.black].hasRested) &&
			      (!players_day[y.white].hasRested || !players_day[y.black].hasRested)) {
			      return true;
			  }
			  if ((players_day[y.white].hasRested || players_day[y.black].hasRested) &&
			      (!players_day[x.white].hasRested || !players_day[x.black].hasRested)) {
			      return false;
			  }
			  return players_day[(uint64_t)x.white].points_per_day[day] +
				     players_day[(uint64_t)x.black].points_per_day[day] <
				 players_day[(uint64_t)y.white].points_per_day[day] +
				     players_day[(uint64_t)y.black].points_per_day[day];
		      });

	    fmt::print("\n");
	    for (Game &g : games) {
		fmt::print("{} - {}, ", g.white, g.black);
	    }
	    fmt::print("\n");

	    while (todayMatches < (num_players - 1) / 2) { // restriccion de partidos por dia (?)
		if (find_best_match(games, gamesPlayed, todayPlayer, day)) {
		    todayMatches++;
		} else {
		    fmt::print("no encontrado {}\n", tries);
		    tries++;
		}
		if (tries > MAX_TRIES) {
		    break;
		}
	    }
	    if (tries > MAX_TRIES) {
		fmt::print("FFFFFFFFFFFF\n");
	    } else
		fmt::print("DAY {} COMPLETED\n", day);
	}
    }

    bool find_best_match(std::vector<Game> &games, std::vector<Game> &gamesPlayed, std::vector<uint64_t> &todayPlayer,
			 uint64_t day) {
	// we search in array games (C) the feasibles matchups and the best suitable
	for (Game &g : games) {
	    if (games.size() == 0) {
		break;
	    }
	    uint64_t white = g.white;
	    uint64_t black = g.black;
	    // fmt::print("LOOKING FOR MATCH\n");

	    // we check that both players didnt play today
	    auto pos = std::find(todayPlayer.begin(), todayPlayer.end(), white);
	    if (pos != todayPlayer.end()) {
		continue;
	    }
	    pos = std::find(todayPlayer.begin(), todayPlayer.end(), black);
	    if (pos != todayPlayer.end()) {
		continue;
	    }

	    // we check that they can play as black/white
	    if (!players[white].can_play_white()) {
		continue;
	    }
	    if (!players[black].can_play_black()) {
		continue;
	    }
	    const Match m{day, white, black};
	    fmt::print("Inserting match {} - {} day {}.\n", white, black, day);
	    matches.insert(m);
	    players[white].games_white++;
	    players[black].games_black++;
	    todayPlayer.push_back(white);
	    todayPlayer.push_back(black);
	    games.erase(std::find(games.begin(), games.end(), g));
	    Game gInverted{black, white};
	    games.erase(std::find(games.begin(), games.end(), gInverted));
	    fmt::print("Deleted from C.\n");
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

    uint64_t lines_read = 0, players_read = 0, number_of_players = 0, remaining_players = UINT64_MAX, player_index = 0;
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
