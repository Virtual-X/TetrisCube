#ifndef STATS_H
#define STATS_H

#include <vector>
#include <sstream>
#include <iostream>

class Stats {
public:
    Stats()
        : count(nc) {}
    void Print() {
        PrintStats();
    }
    void Merge(const Stats& other) {
        for (int i = 0; i < nc; i++)
            count[i] += other.count[i];
    }

    void Next() {
        count[0]++;
    }
    void Increase() {
        count[1]++;
    }
    void Decrease() {
        count[2]++;
    }
    void PosPP() {
        count[3]++;
    }
    void PieceCandidate(int index) {
        count[cc + index]++;
    }
    void ConfigCandidate(int index) {
        count[cc + ic + index]++;
    }
private:

    static const int cc = 4;
    static const int ic = 12;
    static const int vc = 2;
    static const int nc = cc + ic * vc;

#ifdef STATS
    void PrintStats() {
        std::cout << "Stats" << std::endl;
        std::vector<std::string> names{"Next", "Increase", "Decrease", "Pos++",
                                      "PieceCandidate", "ConfigCandidate"};
        for (int i = 0; i < cc; i++)
            std::cout << "    " << names[i] << ": " << Format(count[i]) << std::endl;
        for (int j = 0; j < vc; j++) {
            decltype(count)::value_type n(0);
            for (int i = 0; i < ic; i++) {
                auto nn = count[cc + j * ic + i];
                n += nn;
                std::cout << "    " << names[cc + j] << "[" << i << "]: " << Format(nn) << std::endl;
            }
            std::cout << "    " << names[cc + j] << ": " << Format(n) << std::endl;
        }
    }

    static std::string Format(uint64_t val) {
        std::ostringstream out;
        out << val;
        auto str = out.str();
        std::ostringstream in;
        size_t off = str.size() % 3;
        if (off == 0)
            off += 3;
        in << str.substr(0, off);
        for (size_t i = off; i < str.size(); i += 3)
            in << "'" << str.substr(i, 3);
        return in.str();
    }

    //std::atomic_uint64_t count[nc];
    std::vector<uint64_t> count;
#else
    void PrintStats() {
        std::cout << "Stats not computed" << std::endl;
    }

    struct Slave {
        typedef Slave value_type;
        Slave(int) {}
        inline void operator++ (int) {}
        inline Slave& operator+= (const Slave&) { return *this; }
        inline Slave operator[] (int) const { return *this; }
    };

    Slave count;
#endif
};

#endif // STATS_H
