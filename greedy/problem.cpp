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

struct Player {
    player_id playerID;
    std::vector<uint64_t> points_per_day;
    std::vector<bool> played;
    std::vector<bool> white;
    std::vector<bool> black;
    uint64_t whiteMatches = 0;
    uint64_t blackMatches = 0;

    std::size_t &operator[](std::size_t index) { return points_per_day[index]; }
    std::size_t operator[](std::size_t index) const { return points_per_day[index]; }
};

struct Match {
    uint64_t day;
    player_id white;
    player_id black;

    bool operator<(const Match &other) const {
	return day < other.day;
    }

    bool operator==(const Match &other) const {
        return day == other.day && white == other.white && black == other.black;
    }

};

struct Game {
    player_id white;
    player_id black;

    bool operator==(const Game &other) const {
        return white == other.white && black == other.black;
    }

    bool operator!=(const Game &other) const {
        return white != other.white || black != other.black;
    }
};


struct Tournament {
    std::size_t num_players;
    // We can get the number of days from the
    // number of players
    // std::size_t num_days = num_players;

    std::vector<Player> players;
    std::multiset<Match> matches;
    std::vector<Game> games;

    void set_num_players(std::size_t num_players_) {
	num_players = num_players_;
	players.resize(num_players);
    }

    

    void create_matchups() {
	const uint64_t days = num_players; 
    //we assign the ID to the players
    for(uint64_t player = 0; player < num_players; player++){
        players[player].playerID = player;
        players[player].black.resize(num_players);
        players[player].white.resize(num_players);
        //players[player].played.resize(num_players);
    }


	for(uint64_t day = 0; day < days; day++) {
        fmt::print("DAY {}\n", day);
        
        //sort by ascending points the players each day
        fmt::print("Ordered vector: ");
        std::sort(players.begin(), players.end(), [day] (Player &x, Player &y) { return x.points_per_day[day] < y.points_per_day[day]; });
        for(int i = 0; i < num_players; i++){
            fmt::print("{} ", players[i].playerID);
        }
        fmt::print("\n");
        uint64_t matchCounter = 0;
	    for(uint64_t white = 0; white < num_players; white++) {

            for(uint64_t black = 0; black < num_players; black++) {//TODO ver que si blanco o negro ha jugado hoy no puede volver a jugar

                if(matchCounter == (num_players - 1)/2){//comprueba partidas del dia
                    fmt::print("{} matched plays\n\n", matchCounter);
                    break;
                }

                fmt::print("{} blanco - {} negro\n", players[white].playerID, players[black].playerID);
                fmt::print("\nLISTA DE BLANCOS DEL BLANCO: ");
                
                fmt::print("{} ", players[white].white[day]);
                
                fmt::print("\n BLANCO DE NEGRO: {}", players[white].black[day]);
                fmt::print("\nLISTA DE LOS NEGROS de negro: ");
                
                fmt::print("{} ", players[black].black[day]);

                fmt::print("\n NEGRO DE BLANCO: {}", players[black].white[day]);
                

                if(white != black){//you cannot play vs yourself
                    bool repeated = false;
                    //comprobamos que ninguno de los jugadores haya jugado hoy
                    if(players[white].white[day] || players[white].black[day]){
                        fmt::print("\n\nBLANCO {} ha jugado ya hoy\n\n", players[white].playerID);
                        repeated = true;
                        break;
                    }else if(players[black].black[day] || players[black].white[day]){
                        fmt::print("\n\nNEGRO {} ha jugado ya hoy\n\n", players[black].playerID);
                        repeated = true;
                        //continue; nos ahorramos todo el codigo de dentro
                    }

                    //comprobamos que lleve la mitad de partidas como blanca/negra

                    /*if(day != 0 && (players[white].whiteMatches >= (days + 1)/ 2 || players[black].blackMatches >= (days + 1) / 2)){
                        break;
                    }*/
                    /*int64_t diffW = players[white].whiteMatches - players[white].blackMatches;
                    int64_t diffB = players[black].blackMatches - players[black].blackMatches;
                    if(diffW > 0){
                        fmt::print("Blanco ha jugado demasiado como blanco {}\n", diffW);
                        continue;
                    }

                    if(diffB > 0){
                        fmt::print("Negro ha jugado demasiado como negro {}\n", diffB);
                        continue;
                    }*/


                    const Match m {day, players[white].playerID, players[black].playerID};
                    
                    //we look if that match have been done previously
                    
                    fmt::print("Looking for {} - {}\n", players[white].playerID, players[black].playerID);
                    const Game prevMatch {players[white].playerID, players[black].playerID};
                    const Game prevMatchRev {players[black].playerID, players[white].playerID};
                    

                    //const auto matches_in_day = matches.equal_range(prevMatch);

                    //comparo con los partidos blanco-negro y partidos del reves (blanco era negro y viceversa)
                    auto pos = std::find(games.begin(), games.end(), prevMatch);
                    auto posRev = std::find(games.begin(), games.end(), prevMatchRev);


                    if(pos != games.end()){//miro que no se repita con el normal
                        fmt::print("Match repetido saltamos\n");
                        repeated = true;
                        
                    }else if(posRev != games.end()){//miro que no se repita con el inverso
                        fmt::print("Match Repetido a la inversa saltamos\n");
                        repeated = true;
                        
                    }
                    
                    if(!repeated){//si el match no es repetido lo inserta
                        fmt::print("Insert\n");
                        games.push_back(Game {players[white].playerID, players[black].playerID});
                        matches.insert(m);
                        players[white].white[day] = true;
                        players[white].black[day] = false;
                        players[white].whiteMatches++;
                        players[black].black[day] = true;
                        players[black].white[day] = false;
                        players[black].blackMatches++;
                        matchCounter++;//counter para parar las partidas del dia X en caso de que sobre ya haya los partidos
                        break;
                    }
                    
                    
                }else fmt::print("NO POSSIBLE VS YOURSELF\n");
            }
            if(matchCounter == (num_players - 1)/2){//comprueba partidas del dia
                fmt::print("{} matched plays\n\n", matchCounter);
                
                break;
            }
	    }


	}


	/*
	 *   Players can't play vs themselves
	 *   Every player has to play with every other player
	 *   etc
	 *
	 *   For each day D sort players (increasingly) by the points
	 *   they win by resting this day, and create matchups from this,
	 *   taking into account impossible matchups and so on
	 */
    }

    void print() {
	fmt::print("{} players\n", num_players);
	for (const auto &p : players) {
	    fmt::print("[{}]\n", fmt::join(p.points_per_day, ", "));
	}

	create_matchups();
	fmt::print("[\n\t{}\n]\n", fmt::join(matches, "\n\t"));
    }
};

void get_num_players(const std::string &current_line, uint64_t &number_of_players,
		     uint64_t &remaining_players) {
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

    uint64_t lines_read = 0, players_read = 0, number_of_players = 0,
	     remaining_players = UINT64_MAX, player_index = 0;
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

void print_usage(const char *program_name) {
    fmt::print("Usage: {} instance_filepath\n", program_name);
}

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
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
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
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

    // Check if reached the end of the range:
    if (it != end && *it != '}') throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const Match& m, FormatContext& ctx) const -> decltype(ctx.out()) {
    // ctx.out() is an output iterator to write to.
    return fmt::format_to(ctx.out(), "[Day: {}, White: {}, Black: {}]", m.day, m.white, m.black);
  }
};
