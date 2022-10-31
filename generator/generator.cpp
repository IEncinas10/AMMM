#include "../include/cxxopts.hpp"
#include <algorithm>
#include <cassert>
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <random>
#include <string>

#define MIN_POINTS 0
#define MAX_POINTS 100
#define TOTAL_POINTS 100

static uint64_t N = 0;
static std::mt19937 generator;
static std::uniform_int_distribution<uint16_t> distribution(MIN_POINTS, TOTAL_POINTS);
static bool generate_every_instance = false;
static std::string output_directory;

void setup_generator(uint64_t seed) { generator.seed(seed); }

std::vector<uint16_t> generate_points_per_player(uint64_t players) {
    std::vector<uint16_t> points(players);

    // Let's access the points array via a shuffled array of indices
    // if we don't do that points get clustered into the first
    // positions of the array for every player
    std::vector<uint64_t> indices(players);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), generator);

    uint64_t total = 0;
    for (uint64_t i = 0; i < players && total != TOTAL_POINTS; i++) {
	uint64_t tmp = distribution(generator);

	tmp = std::min(TOTAL_POINTS - total, tmp);
	total += tmp;

	points[indices[i]] = tmp;
    }

    return points;
}

void parse(int argc, char **argv) {
    try {
	cxxopts::Options options(argv[0], "AMMM Course Project 2022\nInstance generator");

	// cxxopts::value<uint32_t>()->default_value("1"))

	// clang-format off
	options.set_width(90).set_tab_expansion().add_options()
	    ("n, numplayers", "Number of players. has to be odd", cxxopts::value<uint64_t>()->default_value("3"))
	    ("s, seed", "Seed for the random engine", cxxopts::value<uint64_t>()->default_value("1"))
	    ("every_instance", "Generate every instance up to [numplayers]")
	    ("output_dir", "Output directory for generated instance(s)", cxxopts::value<std::string>()->default_value("../instances/"))
	    ("h, help", "Print help");
	// clang-format off

	auto result = options.parse(argc, argv);


	if (result.count("help")) {
	    std::cout << options.help() << std::endl;
	    exit(0);
	}

	N                       = result["numplayers"].as<uint64_t>();
	generate_every_instance = result.count("every_instance");
	output_directory        = result["output_dir"].as<std::string>();
	uint64_t seed = result["seed"].as<uint64_t>();
	assert(N % 2 && N > 1);
	
	setup_generator(seed);

	fmt::print("N = {}, Seed = {}, generate_every_instance = {}, output_directory = {}\n",
		N, seed, generate_every_instance, output_directory);

    } catch (const cxxopts::exceptions::exception &e) {
	std::cout << "error parsing options: " << e.what() << std::endl;
	exit(-1);
    }
}

int main(int argc, char **argv) {
    parse(argc, argv);
    
    uint64_t num_players = 3;
    if(!generate_every_instance) 
	num_players = N;
    
    for(; num_players <= N; num_players += 2) {
	const std::string filepath_string = output_directory + fmt::format("project.{}.dat", num_players);

	fmt::print("Generating file {}... ", filepath_string);
	const char *filepath = filepath_string.c_str();
	FILE *file = fopen(filepath, "w+");
	fmt::print(file, "n = {}\n\n\np =\n  [\n", num_players);
	for(uint64_t j = 1; j <= num_players; j++) {
	    const auto tmp = generate_points_per_player(num_players);
	    fmt::print(file, "    [{:<3}]\n", fmt::join(tmp, " "));
	}
	fmt::print(file, "  ];\n");

	fmt::print("done\n");
	fclose(file);
    }
}
