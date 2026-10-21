#include <chrono>
#include <iostream>
#include <string>

#include "../convolution_static.h"

using std::chrono::duration;
using std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_folder> <output_file> <threads>\n";
        return 1;
    }

    std::string inputFolder = argv[1];
    std::string outputFile = argv[2];
    int threads = std::stoi(argv[3]);

    if (!readInput(inputFolder)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    convolveRegionStatic(0, n, 0, m);
    auto time2 = high_resolution_clock::now();

    writeOutput(outputFile);

    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << n << "x" << m << "," << threads << "," << msDouble.count() << "\n";

    return 0;
}