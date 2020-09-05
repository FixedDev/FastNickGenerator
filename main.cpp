#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <functional>
#include <chrono>

std::string &ltrim(std::string &s) {
    auto it = std::find_if(s.begin(), s.end(),
                           [](char c) {
                               return !std::isspace<char>(c, std::locale::classic());
                           });
    s.erase(s.begin(), it);
    return s;
}

std::string &rtrim(std::string &s) {
    auto it = std::find_if(s.rbegin(), s.rend(),
                           [](char c) {
                               return !std::isspace<char>(c, std::locale::classic());
                           });
    s.erase(it.base(), s.end());
    return s;
}

std::string &trim(std::string s) {
    return ltrim(rtrim(s));
}

unsigned long long rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long) hi << 32) | lo;
}

std::function<int()> randomEngine(int start, unsigned long end) {
    std::default_random_engine randomEngine(rdtsc());
    std::uniform_int_distribution<int> distribution(start, end);

    return std::bind(distribution, randomEngine);
}

std::vector<std::string> getFromFile(const std::basic_string<char> &file) {
    std::ifstream dictionary(file);

    std::vector<std::string> strings;
    for (std::array<char, 10> string{}; dictionary.getline(&string[0], 10);) {
        strings.emplace_back(trim(std::string(&string[0])));
    }

    return strings;
}


std::function<std::string()>
stringGeneratorFunction(const std::function<int()> &generator, const std::vector<std::string> &strings) {
    return [generator, strings]() -> std::string {
        int index = generator();
        return strings[index];
    };
}

std::string newNick(
        const std::function<std::string()> &beginning,
        const std::function<std::string()> &middle,
        const std::function<int()> &final) {
    return beginning() + middle() + std::to_string(final());
}

int main() {
    std::vector<std::string> beginningStrings = getFromFile("beginning");
    std::function<int()> beginningGenerator = randomEngine(0, beginningStrings.size() - 1);
    std::function<std::string()> beginning = stringGeneratorFunction(beginningGenerator, beginningStrings);

    std::vector<std::string> middleStrings = getFromFile("middle");
    std::function<int()> middleGenerator = randomEngine(0, middleStrings.size() - 1);
    std::function<std::string()> middle = stringGeneratorFunction(middleGenerator, middleStrings);

    std::function<int()> finalGenerator = randomEngine(0, 9999);

    unsigned long batches;
    std::cout << "Generation batches" << std::endl;
    std::cin >> batches;

    unsigned long nickSize;
    std::cout << "Number of nicks per batch" << std::endl;
    std::cin >> nickSize;
    unsigned long batch = 0;

    auto start = std::chrono::system_clock::now();

    std::ofstream result;
    result.open("result", std::fstream::in | std::fstream::out | std::fstream::app);

    while (batch < batches) {
        std::vector<std::string> nicks;

        unsigned long i = 0;

        while (i < nickSize) {
            std::string nick = newNick(beginning, middle, finalGenerator);
            nicks.push_back(nick);
            i++;
        }


        for (const auto& nick :nicks) {
            result << nick + "\n";
        }

        batch++;
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<float, std::milli> duration = end - start;

    std::cout << "Generated " << nickSize * batches << " in " << duration.count() << "ms" << std::endl;

    return 0;
}
