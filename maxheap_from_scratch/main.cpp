// maxheap_from_scratch.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <stdexcept>
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
// MaxHeap : implémentation "from scratch" (tableau + sift)
// ============================================================
class MaxHeap {
public:
    void reserve(std::size_t n) { m_data.reserve(n); }

    void push(int value) {
        m_data.push_back(value);
        siftUp(m_data.size() - 1);
    }

    int pop() {
        if (m_data.empty())
            throw std::out_of_range("MaxHeap::pop() sur un heap vide");
        const int top = m_data.front();
        m_data.front() = m_data.back();
        m_data.pop_back();
        if (!m_data.empty()) siftDown(0);
        return top;
    }

    int top() const {
        if (m_data.empty())
            throw std::out_of_range("MaxHeap::top() sur un heap vide");
        return m_data.front();
    }

    bool        empty() const noexcept { return m_data.empty(); }
    std::size_t size()  const noexcept { return m_data.size(); }

private:
    void siftUp(std::size_t i) {
        while (i > 0) {
            const std::size_t parent = (i - 1) / 2;
            if (m_data[i] <= m_data[parent]) break;
            std::swap(m_data[i], m_data[parent]);
            i = parent;
        }
    }

    void siftDown(std::size_t i) {
        const std::size_t n = m_data.size();
        while (true) {
            std::size_t largest = i;
            const std::size_t l = 2 * i + 1;
            const std::size_t r = 2 * i + 2;
            if (l < n && m_data[l] > m_data[largest]) largest = l;
            if (r < n && m_data[r] > m_data[largest]) largest = r;
            if (largest == i) break;
            std::swap(m_data[i], m_data[largest]);
            i = largest;
        }
    }

    std::vector<int> m_data;
};

// ============================================================
// Benchmark
// ============================================================
class HeapBenchmark {
public:
    explicit HeapBenchmark(std::string filename) : m_filename(std::move(filename)) {}

    void run() {
        readFile();
        insertAll();
        extractAll();
    }

private:
    // ---------- Étape 1 : lecture du fichier ----------
    void readFile() {
        std::cout << "=== ETAPE 1 : Lecture du fichier (Max-Heap from scratch) ===\n";
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
        std::cout << "=== ETAPE 2 : Insertion dans le Max-Heap ===\n";
        m_heap.reserve(m_data.size());

        m_timer.start();
        for (int v : m_data) m_heap.push(v);
        m_timer.stop();

        const double total = m_timer.elapsedNanoseconds();
        std::cout << "  Nombre d'elements inseres : " << m_heap.size() << "\n";
        std::cout << "  Temps total d'insertion   : " << Timer::format(total) << "\n";
        if (!m_data.empty())
            std::cout << "  Temps moyen par element   : "
                      << Timer::format(total / static_cast<double>(m_data.size())) << "\n";
        if (!m_heap.empty())
            std::cout << "  Max courant (top)         : " << m_heap.top() << "\n";
        std::cout << "\n";
    }

    // ---------- Étape 3 : extraction du max ----------
    void extractAll() {
        std::cout << "=== ETAPE 3 : Extraction du max ===\n";
        if (m_heap.empty()) {
            std::cout << "  Heap vide, rien a extraire.\n";
            return;
        }

        // --- 3 premières extractions mesurées individuellement ---
        std::cout << "  -- 3 premieres extractions (valeurs max) --\n";
        double sumFirst3 = 0.0;
        int    countFirst = 0;
        for (int k = 1; k <= 3 && !m_heap.empty(); ++k) {
            m_timer.start();
            const int v = m_heap.pop();
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
        const std::size_t rest = m_heap.size();
        if (rest > 0) {
            long long checksum = 0; // évite que le compilateur optimise la boucle
            m_timer.start();
            while (!m_heap.empty()) checksum += m_heap.pop();
            m_timer.stop();
            const double bulk = m_timer.elapsedNanoseconds();

            std::cout << "  -- Extraction des " << rest << " elements restants --\n";
            std::cout << "    Temps total : " << Timer::format(bulk) << "\n";
            std::cout << "    Temps moyen : "
                      << Timer::format(bulk / static_cast<double>(rest)) << "\n";
            (void)checksum;
        }
        std::cout << "  Taille finale du heap : " << m_heap.size() << "\n";
    }

    std::string       m_filename;
    std::vector<int>  m_data;
    MaxHeap           m_heap;
    Timer             m_timer;
};

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    const std::string file = (argc > 1) ? argv[1] : "../fichierTXT_Nbre/nombres4.txt";
    HeapBenchmark bench(file);
    bench.run();
    return 0;
}
