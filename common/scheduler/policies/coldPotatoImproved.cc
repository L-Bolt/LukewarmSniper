#include "coldPotatoImproved.h"

#include <iomanip>
using namespace std;
ColdPotatoImproved::ColdPotatoImproved(const PerformanceCounters *performanceCounters,
                         int coreRows, int coreColumns,
                         float criticalTemperature)
    : performanceCounters(performanceCounters),
      coreRows(coreRows),
      coreColumns(coreColumns),
      criticalTemperature(criticalTemperature) {}


std::vector<int> ColdPotatoImproved::map(String taskName, int taskCoreRequirement,
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

std::vector<migration> ColdPotatoImproved::migrate(
    SubsecondTime time, const std::vector<int> &taskIds,
    const std::vector<bool> &activeCores) {

    std::vector<migration> migrations;
    std::vector<bool> availableCores(coreRows * coreColumns);
    std::vector<CoreTemp> temperatures(coreRows * coreColumns);

    for (int c = 0; c < coreRows * coreColumns; c++) {
        availableCores.at(c) = taskIds.at(c) == -1;

        CoreTemp temp;
        temp.temperature = performanceCounters->getTemperatureOfCore(c);
        temp.core_id = c;

        temperatures.at(c) = temp;
    }

    std::stable_sort(
        temperatures.begin(), temperatures.end(),
        [](CoreTemp const& a, CoreTemp const& b) {
            return a.temperature > b.temperature;
        }
    );

    if (temperatures.at(0).temperature > criticalTemperature) {
        cout << "[Scheduler][coldPotatoImproved-migrate]: core" << temperatures.at(0).core_id << " too hot (";
        cout << fixed << setprecision(1) << temperatures.at(0).temperature << ") -> migrate";
        logTemperatures(availableCores);

        if (taskIds.at(temperatures.at(0).core_id) != -1 && taskIds.at(temperatures.at(3).core_id) != -1) {
            migration m;
            m.fromCore = temperatures.at(0).core_id;
            m.toCore = temperatures.at(3).core_id;
            m.swap = true;
            migrations.push_back(m);
            availableCores.at(temperatures.at(3).core_id) = false;
        }
        else if (taskIds.at(temperatures.at(0).core_id) != -1 && taskIds.at(temperatures.at(3).core_id) == -1) {
            migration m;
            m.fromCore = temperatures.at(0).core_id;
            m.toCore = temperatures.at(3).core_id;
            m.swap = false;
            migrations.push_back(m);
            availableCores.at(temperatures.at(3).core_id) = false;
        }


        if (taskIds.at(temperatures.at(1).core_id) != -1 && taskIds.at(temperatures.at(2).core_id) != -1) {
            migration m2;
            m2.fromCore = temperatures.at(1).core_id;
            m2.toCore = temperatures.at(2).core_id;
            m2.swap = true;
            migrations.push_back(m2);
            availableCores.at(temperatures.at(2).core_id) = false;
        }
        else if (taskIds.at(temperatures.at(1).core_id) != -1 && taskIds.at(temperatures.at(2).core_id) == -1) {
            migration m2;
            m2.fromCore = temperatures.at(1).core_id;
            m2.toCore = temperatures.at(2).core_id;
            m2.swap = false;
            migrations.push_back(m2);
            availableCores.at(temperatures.at(2).core_id) = false;
        }
    }

    return migrations;
}

int ColdPotatoImproved::getColdestCore(const std::vector<bool> &availableCores) {
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

int ColdPotatoImproved::getCore(const std::vector<bool> &availableCores) {
    // Get the first free core.
    for (int c = 0; c < coreRows * coreColumns; c++) {
        if (availableCores.at(c)) {
            return c;
        }
    }
    return -1;
}

void ColdPotatoImproved::logTemperatures(const std::vector<bool> &availableCores) {
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
