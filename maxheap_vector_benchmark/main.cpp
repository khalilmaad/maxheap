// vector_benchmark.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
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

class VectorBenchmark {
public:
    explicit VectorBenchmark(std::string filename) : m_filename(std::move(filename)) {}
    void run() { readFile(); insertAll(); extractAll(); }
private:
    void readFile() {
        std::cout << "=== ETAPE 1 : Lecture du fichier (std::vector) ===\n";
        std::ifstream file(m_filename);
        if (!file.is_open()) { std::cerr << "Fichier introuvable: " << m_filename << "\n"; std::exit(EXIT_FAILURE); }
        m_timer.start();
        int v; while (file >> v) m_data.push_back(v);
        m_timer.stop();
        std::cout << "  Nombre d'entiers lus : " << m_data.size() << "\n";
        std::cout << "  Temps de lecture     : " << Timer::format(m_timer.elapsedNanoseconds()) << "\n\n";
    }

    void insertAll() {
        std::cout << "=== ETAPE 2 : Insertion (push_back) ===\n";
        m_vec.reserve(m_data.size());          // equite : evite les realloc
        m_timer.start();
        for (int v : m_data) m_vec.push_back(v);
        m_timer.stop();
        const double total = m_timer.elapsedNanoseconds();
        std::cout << "  Nombre d'elements : " << m_vec.size() << "\n";
        std::cout << "  Temps total       : " << Timer::format(total) << "\n";
        if (!m_data.empty())
            std::cout << "  Temps moyen       : " << Timer::format(total / m_data.size()) << "\n";
        std::cout << "\n";
    }

    int extractMax() {
        auto it = std::max_element(m_vec.begin(), m_vec.end()); // O(n)
        int v = *it;
        m_vec.erase(it);                                        // O(n) : decalage
        return v;
    }

    void extractAll() {
        std::cout << "=== ETAPE 3 : Extraction du max (max_element + erase) ===\n";
        if (m_vec.empty()) { std::cout << "  Rien a extraire.\n"; return; }

        std::cout << "  -- 3 premieres extractions --\n";
        double sum = 0.0; int cnt = 0;
        for (int k = 1; k <= 3 && !m_vec.empty(); ++k) {
            m_timer.start(); int v = extractMax(); m_timer.stop();
            double t = m_timer.elapsedNanoseconds(); sum += t; ++cnt;
            std::cout << "    #" << k << " : " << std::setw(12) << v << " en " << Timer::format(t) << "\n";
        }
        if (cnt) std::cout << "    Moyenne (3) : " << Timer::format(sum / cnt) << "\n";

        const std::size_t rest = m_vec.size();
        if (rest > 0) {
            std::cout << "  -- ATTENTION : " << rest << " extractions O(n) => ~O(n^2) global --\n";
            long long checksum = 0;
            m_timer.start();
            while (!m_vec.empty()) checksum += extractMax();
            m_timer.stop();
            double bulk = m_timer.elapsedNanoseconds();
            std::cout << "    Temps total : " << Timer::format(bulk) << "\n";
            std::cout << "    Temps moyen : " << Timer::format(bulk / rest) << "\n";
            (void)checksum;
        }
        std::cout << "  Taille finale : " << m_vec.size() << "\n";
    }

    std::string       m_filename;
    std::vector<int>  m_data;
    std::vector<int>  m_vec;
    Timer             m_timer;
};

int main(int argc, char** argv) {
    VectorBenchmark b(argc > 1 ? argv[1] : "../fichierTXT_Nbre/nombres4.txt");
    b.run();
    return 0;
}
