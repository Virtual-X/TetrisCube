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

        size_t solutionsCount = 0;

        int i = 0;
        for (auto& t : threads) {
            t = std::thread([=, &mutex, &worked, &solutionsCount] {
                while (true) {
                    size_t job = 0;
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        (void)lock;
                        if (worked >= solvers.size())
                            return;
//                        if (solutionsCount > 5000)
//                            return;
                        job = worked;
                        worked++;
                    }
                    function(*solvers[job], i, job);
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        (void)lock;
                        solutionsCount += solvers[job]->solutions.size();
                    }
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
