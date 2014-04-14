#ifndef PARALLELSOLVER_H
#define PARALLELSOLVER_H

#include <thread>
#include <memory>
#include <functional>
#include <mutex>

template<class TSolver>
class ParallelSolver {
public:
    ParallelSolver(const TSolver& solver, int splitLevel) {
        solvers = solver.Split(splitLevel);
    }

    size_t GetSplitCount() const {
        return solvers.size();
    }

    std::list<Solution> Solve(std::function<void(TSolver&,int,int)> function, int nthreads) {
        std::vector<std::thread> threads(nthreads);

        std::mutex mutex;
        size_t worked = 0;
        const size_t solversCount = solvers.size();

        int i = 0;
        for (auto& t : threads) {
            t = std::thread([=, &mutex, &worked] {
                while (true) {
                    size_t job = 0;
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        (void)lock;
                        if (worked >= solversCount)
                            return;
                        job = worked;
                        worked++;
                    }
                    function(*solvers[job], i, job);
                }
            });
            i++;
        }

        for (auto& t : threads) {
            t.join();
        }
        std::list<Solution> solutions;
        for (auto& s : solvers) {
            solutions.insert(solutions.end(), s->solutions.begin(), s->solutions.end());
        }
        return solutions;
    }

private:
    std::vector<std::shared_ptr<TSolver>> solvers;
};

#endif // PARALLELSOLVER_H
