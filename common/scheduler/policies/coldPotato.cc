#include "coldPotato.h"

#include <iomanip>
using namespace std;
ColdPotato::ColdPotato(const PerformanceCounters *performanceCounters,
                         int coreRows, int coreColumns,
                         float criticalTemperature)
    : performanceCounters(performanceCounters),
      coreRows(coreRows),
      coreColumns(coreColumns),
      criticalTemperature(criticalTemperature) {}


std::vector<int> ColdPotato::map(String taskName, int taskCoreRequirement,
                                  const std::vector<bool> &availableCoresRO,
                                  const std::vector<bool> &activeCores) {
    std::vector<bool> availableCores(availableCoresRO);
    std::vector<int> cores;
    logTemperatures(availableCores);
    for (; taskCoreRequirement > 0; taskCoreRequirement--) {
        int coldestCore = getCore(availableCores);
        if (coldestCore == -1) {
            // not enough free cores
            std::vector<int> empty;
            return empty;
        } else {
            cores.push_back(coldestCore);
            availableCores.at(coldestCore) = false;
        }
    }
    return cores;
}

int ColdPotato::getFirstFreeCore(std::vector<bool> &availableCores) {
    for (int i = 0; i < 4; i++) {
        if (availableCores.at(i)) {
            return i;
        }
    }

    return -1;
}


std::vector<migration> ColdPotato::migrate(
    SubsecondTime time, const std::vector<int> &taskIds,
    const std::vector<bool> &activeCores) {

    std::vector<migration> migrations;
    std::vector<bool> availableCores(coreRows * coreColumns);
    for (int c = 0; c < coreRows * coreColumns; c++) {
        availableCores.at(c) = taskIds.at(c) == -1;
    }

    int firstFreeCore = getFirstFreeCore(availableCores);

    for (int c = 0; c < coreRows * coreColumns; c++) {
        if (activeCores.at(c)) {
            float temperature = performanceCounters->getTemperatureOfCore(c);
            cout << "[Scheduler][coldPotato-migrate]: core" << c << " temperature (";
            cout << fixed << setprecision(1) << temperature << ") -> migrate";
            logTemperatures(availableCores);

            migration m;
            int next_core = (4 + ((firstFreeCore - 1) % 4)) % 4;
            m.fromCore = next_core;
            m.toCore = firstFreeCore;
            m.swap = false;
            migrations.push_back(m);
            availableCores.at(firstFreeCore) = false;
            availableCores.at(next_core) = true;
            firstFreeCore = next_core;
        }
    }
    return migrations;
}

int ColdPotato::getColdestCore(const std::vector<bool> &availableCores) {
    int coldestCore = -1;
    float coldestTemperature = 0;
    // iterate all cores to find coldest
    for (int c = 0; c < coreRows * coreColumns; c++) {
        if (availableCores.at(c)) {
            float temperature = performanceCounters->getTemperatureOfCore(c);
            if ((coldestCore == -1) || (temperature < coldestTemperature)) {
                coldestCore = c;
                coldestTemperature = temperature;
            }
        }
    }
    return coldestCore;
}

int ColdPotato::getCore(const std::vector<bool> &availableCores) {
    // Get the first free core.
    for (int c = 0; c < coreRows * coreColumns; c++) {
        if (availableCores.at(c)) {
            return c;
        }
    }
    return -1;
}

void ColdPotato::logTemperatures(const std::vector<bool> &availableCores) {
    cout << "[Scheduler][coldestCore-map]: temperatures of available cores:"
         << endl;
    for (int y = 0; y < coreRows; y++) {
        for (int x = 0; x < coreColumns; x++) {
            if (x > 0) {
                cout << " ";
            }
            int coreId = y * coreColumns + x;
            if (!availableCores.at(coreId)) {
                cout << " - ";
            } else {
                float temperature =
                    performanceCounters->getTemperatureOfCore(coreId);
                cout << fixed << setprecision(1) << temperature;
            }
        }
        cout << endl;
    }
}
