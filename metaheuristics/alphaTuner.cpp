#include "../include/cxxopts.hpp"
#include "algorithms.h"

static uint64_t MAXINSTANCES = 0;
static const uint64_t MAXPOINTS = 100;
static std::vector<uint64_t> alpha_solution;

void parse(int argc, char **argv) {
    try {
	cxxopts::Options options(argv[0], "AMMM Course Project 2022\nAlpha tuner");

	// clang-format off
	options.set_width(90).set_tab_expansion().add_options()
	    ("instances", "Range from {3..arg} with step 2", cxxopts::value<uint64_t>())
	    ("h, help", "Print help");
	// clang-format on

	auto result = options.parse(argc, argv);

	if (result.count("help") || !result.count("instances")) {
	    std::cout << options.help() << std::endl;
	    exit(0);
	}
	MAXINSTANCES = result["instances"].as<uint64_t>();
	print_info = false;

    } catch (const cxxopts::exceptions::exception &e) {
	std::cout << "error parsing options: " << e.what() << std::endl;
	exit(-1);
    }
}

/***************************************************************/
/*******************READING OPTIMAL SOLUTIONS*******************/
/***************************************************************/

bool read_optimal_solutions(const char *results_filename, std::vector<uint64_t> &optimal_solutions,
			    std::vector<double> &ilp_time) {
    std::ifstream input(results_filename);
    if (!input.is_open())
	return false;

    uint64_t lines_read = 0;
    std::string current_line;

    double time;
    uint64_t score;
    std::string tmp;
    char dummy;

    do {
	std::getline(input, current_line);
	std::istringstream iss(current_line);

	iss >> tmp >> dummy >> time >> dummy >> score;

	optimal_solutions.push_back(score);
	ilp_time.push_back(time);

	lines_read++;

    } while (lines_read < ((MAXINSTANCES - 1) / 2));

    return true;
}

double arith_mean(const std::vector<double> &errors) {
    if (errors.size() == 0)
	return +INFINITY;
    double total = 0;
    for (int i = 0; i < errors.size(); i++) {
	total += errors[i];
    }

    return total / errors.size();
}

int main(int argc, char **argv) {
    generator.seed(0);

    parse(argc, argv);

    std::vector<uint64_t> optimal_solutions; // read from results...
    std::vector<double> mean_error_by_alpha;

    double best_alpha = -1;
    double best_alpha_error = +INFINITY;

    std::vector<double> ilp_times;
    std::string prefix = "../instances/project.";
    std::string sufix = ".dat";

    const char *results_filename = "../results/ilp/clean";

    bool read_solution_ok = read_optimal_solutions(results_filename, optimal_solutions, ilp_times);
    assert(read_solution_ok);

    // para cada alpha probar todas las instances
    for (int j = 0; j <= MAXPOINTS; j++) {
	alpha = (float)j / MAXPOINTS;
	std::vector<double> errors_alpha;
	alpha_solution.clear();

	useLocalSearch = true;

	uint64_t index = 0;
	for (int i = 3; i <= MAXINSTANCES; i += 2) {
	    std::string instance_filename_str = prefix + std::to_string(i) + sufix;
	    const char *instance_filename = instance_filename_str.c_str();
	    Tournament tournament;

	    bool read_ok = read_instance(instance_filename, tournament);
	    assert(read_ok);

	    uint64_t solution = tournament.create_matchups();
	    alpha_solution.push_back(solution);

	    errors_alpha.push_back(
		abs(static_cast<int>(optimal_solutions[index] - alpha_solution[index])));

	    index++;
	}
	fmt::print("{}\n", j);
	fmt::print("[{}]\n", fmt::join(errors_alpha, ", "));

	double mean_error = arith_mean(errors_alpha);

	mean_error_by_alpha.push_back(mean_error);

	// fmt::print("Alpha = {}, error= {}", alpha, mean_error);

	if (mean_error < best_alpha_error) {
	    best_alpha = alpha;
	    best_alpha_error = mean_error;
	}
    }

    fmt::print("\n");

    fmt::print("[{}]\n", fmt::join(mean_error_by_alpha, ", "));

    fmt::print("\nBEST ALPHA FOR INSTANCES IS {:.32} WITH ERROR: {}\n\n\n", best_alpha,
	       best_alpha_error);

    for (int j = 0; j <= MAXPOINTS; j++) {
	alpha = (float)j / MAXPOINTS;
	fmt::print("{}, {}\n", alpha, mean_error_by_alpha[j]);
    }
}
