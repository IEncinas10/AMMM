#include "../include/cxxopts.hpp"
#include "algorithms.h"

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

	if (argc < 2 || result.count("help")) {
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

    fmt::print("Alpha: {}\n", alpha);

    const char *instance_filename = instance_filename_str.c_str();

    Tournament tournament;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    bool read_ok = read_instance(instance_filename, tournament);
    assert(read_ok);
    tournament.create_matchups();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time (s): "
	      << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() /
		     1000000000.0
	      << std::endl;

    if (print_calendar)
	tournament.print();
}
