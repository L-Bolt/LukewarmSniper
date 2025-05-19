/**
* This header implements a policy that maps new applications to the coldest
core
* and migrates threads from hot cores to the coldest cores.
*/
#ifndef __COLDPOTATO_H
#define __COLDPOTATO_H
#include <vector>

#include "mappingpolicy.h"
#include "migrationpolicy.h"
#include "performance_counters.h"
class ColdPotato : public MappingPolicy, public MigrationPolicy {
   public:
   ColdPotato(const PerformanceCounters *performanceCounters, int coreRows,
                int coreColumns, float criticalTemperature);
    virtual std::vector<int> map(String taskName, int taskCoreRequirement,
                                 const std::vector<bool> &availableCores,
                                 const std::vector<bool> &activeCores);
    virtual std::vector<migration> migrate(
        SubsecondTime time, const std::vector<int> &taskIds,
        const std::vector<bool> &activeCores);

   private:
    const PerformanceCounters *performanceCounters;
    unsigned int coreRows;
    unsigned int coreColumns;
    float criticalTemperature;
    int getColdestCore(const std::vector<bool> &availableCores);
    int getCore(const std::vector<bool> &availableCores);
    void logTemperatures(const std::vector<bool> &availableCores);
    uint64_t prevTime = -1;
};
#endif

