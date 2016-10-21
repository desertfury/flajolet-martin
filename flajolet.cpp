#include <bitset>
#include <cmath>
#include <iostream>
#include <functional>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>

constexpr size_t R = 64;

std::vector<std::vector<size_t>> getHashFunctions(size_t functions_count, size_t radix) {
	std::vector<std::vector<size_t>> result;
	result.reserve(functions_count);
	for (size_t i = 0; i < functions_count; ++i) {
		std::vector<size_t> func;
		func.reserve(radix);
		for (size_t h = 0; h < radix; ++h) {
			size_t m = 0;
			while (true) {
				m = size_t(rand());
				if (m % 2 == 0) {
					break;
				}
			}
			func.push_back(m);
		}
		result.push_back(func);
	}
	return result;
}

template <typename T>
class FMCardinalitySolver {
private:
	std::bitset<R> bset;
	std::function<size_t(const T&)> hashfunc;
	static constexpr double phi = 0.77351;

	size_t findFirstSignBit(size_t value) const {
		value >>= 1;
		size_t i = 1;
		while (!(value & 1) && i < R) {
			value >>= 1;
			++i;
		}
		return i;
	}

public:
	FMCardinalitySolver(const std::function<size_t(const T&)> &f) : bset(), hashfunc(f) {
	};

	void Add(const T& value) {
		size_t hash = hashfunc(value);
		bset.set(findFirstSignBit(hash));
	}

	size_t Cardinality() const {
		size_t i = 0;
		for (i = R; i > 0; --i) {
			if (bset[i]) {
				break;
			}
		}
		if (i == R) {
			i = 0;
		}
		return ceil(double(1<<i) / phi);
	}
};

template <typename T>
class MedianFMCardinalitySolver {
private:
	std::vector<FMCardinalitySolver<T>> solvers;
	std::vector<std::function<size_t(const T&)>> hashfuncs;

public:
	MedianFMCardinalitySolver(const std::vector<std::function<size_t(const T&)>> v_f) : solvers(), hashfuncs(v_f) {
		for (const auto &f : hashfuncs) {
			solvers.push_back(FMCardinalitySolver<T>(f));
		}
	};

	void Add(const T& value) {
		for (auto& s : solvers) {
			s.Add(value);
		}
	}

	size_t Cardinality() const {
		std::vector<size_t> estimates;
		for (auto& s : solvers) {
			estimates.push_back(s.Cardinality());
		}
		std::sort(estimates.begin(), estimates.end());
		return estimates[estimates.size() / 2];
	}
};

template <typename T>
class NaiveSolver {
	std::unordered_set<T> s;
public:
	void Add(const T& value) {
		s.insert(value);
	}

	size_t Cardinality() const {
		return s.size();
	}
};

template <typename Solver>
void processFile(const std::string &filename, Solver &solver) {
	std::ifstream input(filename.c_str(), std::ifstream::in);
	for (std::string line; std::getline(input, line); ) {
		std::istringstream iss(line);
		while (true) {
			if (!iss)
				break;
			std::string sub;
			iss >> sub;
			if (sub.size()) {
				solver.Add(sub);
			}
		}
	}
}

int main(int argc, char **argv) {
	std::srand(13327);
	if (argc < 2)
		return 0;
	auto StringHash = std::hash<std::string>();
	FMCardinalitySolver<std::string> solver(StringHash);
	NaiveSolver<std::string> naive;
	processFile(argv[1], naive);
	processFile(argv[1], solver);
	std::cout << "naive " << naive.Cardinality() << std::endl;
	std::cout << "flajolet " << solver.Cardinality() << std::endl;

	size_t total_funcs = 50;
	auto vecs = getHashFunctions(total_funcs, R);
	std::vector<std::function<size_t(const std::string &)>> hfuncs;
	for (size_t i = 0; i < total_funcs; ++i) {
		auto func = [&vecs, i, &StringHash] (const std::string &text) {
			const auto &vec = vecs[i];
			size_t hash = StringHash(text);
			return vec[hash % R];
		};
		hfuncs.push_back(func);
	}

	MedianFMCardinalitySolver<std::string> mean_solver(hfuncs);
	processFile(argv[1], mean_solver);
	std::cout << "mean flajolet " << mean_solver.Cardinality() << std::endl;

	return 0;
}
