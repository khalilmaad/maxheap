// maxheap_stl.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstdlib>

// ============================================================
// Timer : mesure de temps haute résolution + formatage
// ============================================================
class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;

    void start() { m_start = Clock::now(); }
    void stop()  { m_end   = Clock::now(); }

    double elapsedNanoseconds() const {
        return std::chrono::duration<double, std::nano>(m_end - m_start).count();
    }

    static std::string format(double ns) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
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

// ============================================================
// Benchmark : utilise std::priority_queue<int> (max-heap STL)
// ============================================================
class StlHeapBenchmark {
public:
    explicit StlHeapBenchmark(std::string filename) : m_filename(std::move(filename)) {}

    void run() {
        readFile();
        insertAll();
        extractAll();
    }

private:
    // ---------- Étape 1 : lecture ----------
    void readFile() {
        std::cout << "=== ETAPE 1 : Lecture du fichier (Max-Heap STL) ===\n";
        std::ifstream file(m_filename);
        if (!file.is_open()) {
            std::cerr << "Impossible d'ouvrir le fichier: " << m_filename << "\n";
            std::exit(EXIT_FAILURE);
        }

        m_timer.start();
        int v;
        while (file >> v) m_data.push_back(v);
        m_timer.stop();

        std::cout << "  Nombre d'entiers lus : " << m_data.size() << "\n";
        std::cout << "  Temps de lecture     : "
                  << Timer::format(m_timer.elapsedNanoseconds()) << "\n\n";
    }

    // ---------- Étape 2 : insertion ----------
    void insertAll() {
        std::cout << "=== ETAPE 2 : Insertion dans le priority_queue ===\n";

        m_timer.start();
        for (int v : m_data) m_pq.push(v);
        m_timer.stop();

        const double total = m_timer.elapsedNanoseconds();
        std::cout << "  Nombre d'elements inseres : " << m_pq.size() << "\n";
        std::cout << "  Temps total d'insertion   : " << Timer::format(total) << "\n";
        if (!m_data.empty())
            std::cout << "  Temps moyen par element   : "
                      << Timer::format(total / static_cast<double>(m_data.size())) << "\n";
        if (!m_pq.empty())
            std::cout << "  Max courant (top)         : " << m_pq.top() << "\n";
        std::cout << "\n";
    }

    // ---------- Étape 3 : extraction du max ----------
    void extractAll() {
        std::cout << "=== ETAPE 3 : Extraction du max ===\n";
        if (m_pq.empty()) {
            std::cout << "  Heap vide, rien a extraire.\n";
            return;
        }

        // --- 3 premières extractions mesurées individuellement ---
        std::cout << "  -- 3 premieres extractions (valeurs max) --\n";
        double sumFirst3 = 0.0;
        int    countFirst = 0;
        for (int k = 1; k <= 3 && !m_pq.empty(); ++k) {
            m_timer.start();
            const int v = m_pq.top();
            m_pq.pop();
            m_timer.stop();
            const double t = m_timer.elapsedNanoseconds();
            sumFirst3 += t;
            ++countFirst;
            std::cout << "    Extraction #" << k << " : " << std::setw(12) << v
                      << " en " << Timer::format(t) << "\n";
        }
        if (countFirst > 0)
            std::cout << "    Temps moyen (3) : "
                      << Timer::format(sumFirst3 / countFirst) << "\n";

        // --- Extraction en bloc du reste ---
        const std::size_t rest = m_pq.size();
        if (rest > 0) {
            long long checksum = 0;
            m_timer.start();
            while (!m_pq.empty()) {
                checksum += m_pq.top();
                m_pq.pop();
            }
            m_timer.stop();
            const double bulk = m_timer.elapsedNanoseconds();

            std::cout << "  -- Extraction des " << rest << " elements restants --\n";
            std::cout << "    Temps total : " << Timer::format(bulk) << "\n";
            std::cout << "    Temps moyen : "
                      << Timer::format(bulk / static_cast<double>(rest)) << "\n";
            (void)checksum;
        }
        std::cout << "  Taille finale du heap : " << m_pq.size() << "\n";
    }

    std::string              m_filename;
    std::vector<int>         m_data;
    std::priority_queue<int> m_pq;      // max-heap par défaut
    Timer                    m_timer;
};

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    const std::string file = (argc > 1) ? argv[1] : "../fichierTXT_Nbre/nombres4.txt";
    StlHeapBenchmark bench(file);
    bench.run();
    return 0;
}
