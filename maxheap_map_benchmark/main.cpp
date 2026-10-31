// map_benchmark.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <iterator>
#include <cstdlib>

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    void start() { m_start = Clock::now(); }
    void stop()  { m_end   = Clock::now(); }
    double elapsedNanoseconds() const {
        return std::chrono::duration<double, std::nano>(m_end - m_start).count();
    }
    static std::string format(double ns) {
        std::ostringstream oss; oss << std::fixed << std::setprecision(3);
        if      (ns < 1'000.0)          oss << ns                    << " ns";
        else if (ns < 1'000'000.0)      oss << ns / 1'000.0          << " us";
        else if (ns < 1'000'000'000.0)  oss << ns / 1'000'000.0      << " ms";
        else if (ns < 60'000'000'000.0) oss << ns / 1'000'000'000.0  << " s";
        else                            oss << ns / 60'000'000'000.0 << " min";
        return oss.str();
    }
private:
    Clock::time_point m_start{}, m_end{};
};

class MapBenchmark {
public:
    explicit MapBenchmark(std::string filename) : m_filename(std::move(filename)) {}
    void run() { readFile(); insertAll(); extractAll(); }
private:
    void readFile() {
        std::cout << "=== ETAPE 1 : Lecture du fichier (std::map) ===\n";
        std::ifstream file(m_filename);
        if (!file.is_open()) { std::cerr << "Fichier introuvable: " << m_filename << "\n"; std::exit(EXIT_FAILURE); }
        m_timer.start();
        int v; while (file >> v) m_data.push_back(v);
        m_timer.stop();
        std::cout << "  Nombre d'entiers lus : " << m_data.size() << "\n";
        std::cout << "  Temps de lecture     : " << Timer::format(m_timer.elapsedNanoseconds()) << "\n\n";
    }

    void insertAll() {
        std::cout << "=== ETAPE 2 : Insertion dans std::map ===\n";
        m_timer.start();
        for (int v : m_data) m_map.emplace(v, true);  // doublons ignores
        m_timer.stop();
        const double total = m_timer.elapsedNanoseconds();
        std::cout << "  Nombre de cles dans le map : " << m_map.size() << "\n";
        std::cout << "  Temps total d'insertion    : " << Timer::format(total) << "\n";
        if (!m_data.empty())
            std::cout << "  Temps moyen par element    : " << Timer::format(total / m_data.size()) << "\n";
        if (!m_map.empty()) std::cout << "  Max courant (rbegin->first): " << m_map.rbegin()->first << "\n";
        std::cout << "\n";
    }

    int extractMax() {
        auto it = std::prev(m_map.end());
        int v = it->first;
        m_map.erase(it);
        return v;
    }

    void extractAll() {
        std::cout << "=== ETAPE 3 : Extraction du max (rbegin + erase) ===\n";
        if (m_map.empty()) { std::cout << "  Map vide.\n"; return; }

        std::cout << "  -- 3 premieres extractions --\n";
        double sum = 0.0; int cnt = 0;
        for (int k = 1; k <= 3 && !m_map.empty(); ++k) {
            m_timer.start(); int v = extractMax(); m_timer.stop();
            double t = m_timer.elapsedNanoseconds(); sum += t; ++cnt;
            std::cout << "    #" << k << " : " << std::setw(12) << v << " en " << Timer::format(t) << "\n";
        }
        if (cnt) std::cout << "    Moyenne (3) : " << Timer::format(sum / cnt) << "\n";

        const std::size_t rest = m_map.size();
        if (rest > 0) {
            long long checksum = 0;
            m_timer.start();
            while (!m_map.empty()) checksum += extractMax();
            m_timer.stop();
            double bulk = m_timer.elapsedNanoseconds();
            std::cout << "  -- Extraction des " << rest << " elements restants --\n";
            std::cout << "    Temps total : " << Timer::format(bulk) << "\n";
            std::cout << "    Temps moyen : " << Timer::format(bulk / rest) << "\n";
            (void)checksum;
        }
        std::cout << "  Taille finale : " << m_map.size() << "\n";
    }

    std::string           m_filename;
    std::vector<int>      m_data;
    std::map<int, bool>   m_map;
    Timer                 m_timer;
};

int main(int argc, char** argv) {
    MapBenchmark b(argc > 1 ? argv[1] : "../fichierTXT_Nbre/nombres4.txt");
    b.run();
    return 0;
}
