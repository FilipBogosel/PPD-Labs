#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using std::chrono::duration;
using std::chrono::high_resolution_clock;

std::vector<double> addVectors(const std::vector<double>& a, const std::vector<double>& b) {
    std::vector<double> c(a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        c[i] = a[i] + b[i];
    }
    return c;
}

bool readInput(const std::string& filename, size_t& n, std::vector<double>& a, std::vector<double>& b) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open input file " << filename << "\n";
        return false;
    }
    in >> n;
    a.resize(n);
    b.resize(n);
    for (size_t i = 0; i < n; ++i) {
        in >> a[i];
    }
    for (size_t i = 0; i < n; ++i) {
        in >> b[i];
    }
    return true;
}

bool writeOutput(const std::string& filename, size_t n, const std::vector<double>& c) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open output file " << filename << "\n";
        return false;
    }
    out << n << "\n";
    out << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < n; ++i) {
        out << c[i] << " ";
    }
    out << "\n";
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <threads>\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    int threads = std::stoi(argv[3]);

    size_t n = 0;
    std::vector<double> a, b;
    if (!readInput(inputFile, n, a, b)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    std::vector<double> c = addVectors(a, b);
    auto time2 = high_resolution_clock::now();

    writeOutput(outputFile, n, c);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << n << "," << threads << "," << msDouble.count() << "\n";
    return 0;
}