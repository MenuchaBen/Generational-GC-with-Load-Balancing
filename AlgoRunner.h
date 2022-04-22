/*
 *	Created by Eyal Lotan and Dor Sura.
 */

#ifndef FLASHGC_ALGORUNNER_H
#define FLASHGC_ALGORUNNER_H

#include "MyRand.h"
#include "FTL.hpp"
#include "ListItem.h"
#include "Auxilaries.h"
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_set>
 //#include <unistd.h>

#define TBD -11

using std::map;
using std::vector;

typedef enum {
    BASIC_GEN, GENx_STATIC_BOUND, GENx_LOAD_BALANCE_OPT, GENx_LOAD_BALANCE_MANUALY
} GenAlgoType;

class UserParameters {
public:
    unsigned long long window_size;
    int number_of_generations;
    GenAlgoType generational_type;
    /* parameters for Hot/Cold memory simulation */
    int hot_pages_percentage;
    double hot_pages_probability;
};

class AlgoRunner {
public:

    ////// member elements:  //////
    /* Algorithm's type which user would like to simulate */
    Algorithm algo;

    /* the writing sequence that is given as an input to all Look Ahead algorithms in this class.
     * the writing sequence is an array of integers, where writing_sequence[i] is the logical page
     * number that will be written in the ith place (i.e the i+1 write since we start from 0).
     */
    unsigned int* writing_sequence;

    /* number of pages in writing sequence. This parameter can be adjusted to be a window of known writes, but
     * this feature may require some more adjustments
     * In the current implementation we use the global NUMBER_OF_PAGES macro, but this is bad practice for sure
     * if we wish to scale up in any way.. in that case we should switch and use this member element */
    unsigned long long number_of_pages;

    unsigned long long* k_bounds;
    unsigned long long* physical_writes_per_gen;
    unsigned long long* logical_writes_per_gen;


    unsigned long long interval;
    int* gens_population;

    /* location list is a hash table that maps a logical page number to a ListItem that contains a list of indexes
     * in the writing sequence such that for page i and each index j in the location list matching that page i it
     * holds that writing_sequence[j] == i
     */
    map<unsigned int, ListItem>* logical_pages_location_map;

    /* writing page_dist represents the data distribution type - uniform distribution or Hot/Cold distribution */
    PageDistribution page_dist;

    /* class which contains:
     * HOT_COLD parameters (hot pages percentage and probability)
     * window size
     * num of generations for generational algorithm
     */
    UserParameters user_parameters;

    WindowSizeFlag window_size_flag;

    /* FTL memory layout object */
    FTL* ftl;

    /* data to write in each page. As mentioned below, this data is generated randomly and is the same across all
     * pages. for the sake if this simulator this is fine, but of course you can change this to contain some
     * meaningful data
     */
    char* data;

    bool reach_steady_state;
    bool print_mode;

    list<LogicalPage**> gc_pages_to_rewrite;

    ////// C'tors & D'tor:  //////

    /* C'tor for scheduledGC object.
     * NOTE: throughout the whole class implementation we use the global macros NUMBER_OF_PAGES, PAGES_PER_BLOCK, etc.
     * I guess that best practice is not to use the global macros, and instead pass relevant parameters to the class c'tor (as we do with number_of_pages).
     * but for now this is fine. If you were to use this class file and copy it to their your personal use,
     * you should note that some changes may be needed to use only the parameters passed to the class
     * c'tor (and this is better coding practice).
     */
    AlgoRunner(long long number_of_pages, PageDistribution page_dist, Algorithm algo, WindowSizeFlag window_size_flag) :
        algo(algo), number_of_pages(number_of_pages), k_bounds(nullptr), interval(NUMBER_OF_PAGES), gens_population(nullptr),
        page_dist(page_dist), window_size_flag(window_size_flag), ftl(nullptr),
        data(nullptr), reach_steady_state(true), print_mode(false), gc_pages_to_rewrite({}) {
        /* generates writing sequence for uniform or hot-cold distribution */
        generateWritingSequence();

        /* construct a mapping between a logical page number to a ListItem corresponding the logical page.
        * each ListItem for logical page i contains the list of locations in the writing sequence where
        * the page i is written. each ListItem contains a list of locations sorted in an ascending order.
        */

        logical_pages_location_map = createLocationsMap(0, NUMBER_OF_PAGES);
        initializeFTL();

        /* get extra parameters:
         * window size
         * num of generations for generational algorithm
        */
        getUserParams();

        if (algo == GENERATIONAL) {
            if (user_parameters.generational_type == GENx_LOAD_BALANCE_MANUALY) {
                interval = BETA;
            }
            else if (user_parameters.generational_type == GENx_LOAD_BALANCE_OPT) {
                interval = 0.2 * LOGICAL_BLOCK_NUMBER * PAGES_PER_BLOCK;
            }
            //else: interval remains N - initialized in constructor of AlgoRunner
            if (user_parameters.generational_type == GENx_LOAD_BALANCE_MANUALY || 
                user_parameters.generational_type == GENx_LOAD_BALANCE_OPT) {
                gens_population = new int[user_parameters.number_of_generations];
                if (user_parameters.generational_type == GENx_LOAD_BALANCE_OPT) {
                    if (user_parameters.number_of_generations == 2) {
                        double op = (PHYSICAL_BLOCK_NUMBER - LOGICAL_BLOCK_NUMBER) / LOGICAL_BLOCK_NUMBER;
                        if (op < 0.1) {
                            gens_population[0] = round(0.45 * interval);
                        }
                        else if (op < 0.15) {
                            gens_population[0] = round(0.5 * interval);
                        }
                        else if (op < 0.35) {
                            gens_population[0] = round(0.55 * interval);
                        }
                        else if (op < 0.7) {
                            gens_population[0] = round(0.6 * interval);
                        }
                        else {
                            gens_population[0] = round(0.65 * interval);
                        }
                        gens_population[1] = interval - gens_population[0];
                    }
                    else {
                        //TODO: complete for more than 2 generations
                    }
                }
                else if (user_parameters.generational_type == GENx_LOAD_BALANCE_MANUALY) {
                    cout << "Enter gen population:" << endl;
                    double count_to_one = 0;
                    unsigned long long cout_to_interval = 0;
                    for (int i = 0; i < user_parameters.number_of_generations; i++) {
                        double precentage_pop;
                        cin >> precentage_pop;
                        count_to_one += precentage_pop;
                        if (i < user_parameters.number_of_generations - 1) {
                            gens_population[i] = round(precentage_pop * interval);
                            cout_to_interval += gens_population[i];
                        }
                        else if (count_to_one == 1) {
                            gens_population[user_parameters.number_of_generations - 1] = interval - cout_to_interval;
                        }
                        else {
                            cerr << "Load balancing incorrent" << endl;
                            exit(-1);
                        }
                    }
                    
                }
            }
            initializeKBounds();
            initializePagesPerGenCounters();
        }
    }

    ~AlgoRunner() {
        if (algo == GENERATIONAL) {
            delete[] k_bounds;
            delete[] physical_writes_per_gen;
            delete[] logical_writes_per_gen;
            if (gens_population) {
                delete[] gens_population;
            }
        }
        delete[] writing_sequence;
        delete[] logical_pages_location_map;
        delete[] data;
        delete ftl;
    }


    ////// class functions:  //////


    /* initialize FTL simulator. We construct an FTL object that represents the memory layout.
     * If steady state flag is turned on, we bring the simulator to a steady state by writing random pages
     * (we choose the pages uniformly in random).
     * NOTE: the data is generated by random. for the purpose of this simulator there is no meaning to the
     * data itself. for this reason we populate all pages with the same value.
     */
    void initializeFTL() {
        ftl = new FTL();

        /* initialize data page. will remain the same */
        data = new char[PAGE_SIZE];

        /* fill pages with random data */
        for (int j = 0; j < PAGE_SIZE; j++) {
            data[j] = KISS() % 256;
        }
    }

    void setSteadyState(bool state) {
        reach_steady_state = state;
    }

    void setPrintMode(bool mode) {
        print_mode = mode;
        ftl->print_mode = mode;
    }

    void reachSteadyState() {
        unsigned int logical_page_to_write;
        data = new char[PAGE_SIZE];

        /* fill pages with random data */
        for (int j = 0; j < PAGE_SIZE; j++) {
            data[j] = KISS() % 256;
        }

        /* reach steady state */
        cout << "Reaching Steady State..." << endl;
        if (print_mode) {
            ftl->printHeader();
        }
        /* you can adjust this */
        for (int i = 0; i < 1000000; i++) {
            logical_page_to_write = KISS() % (LOGICAL_BLOCK_NUMBER * PAGES_PER_BLOCK);
            ftl->write(data, logical_page_to_write, GREEDY);
        }
        ftl->erases_steady = ftl->erases;
        ftl->logicalPageWritesSteady = ftl->logicalPageWrites;
        ftl->physicalPageWritesSteady = ftl->physicalPageWrites;
        cout << "Steady State Reached..." << endl;
        cout << endl;
    }

    /* get the number of unique logical pages in writing_sequence */
    unsigned int getLocationListSize(unsigned long long base_index, unsigned int window_size) const {
        set<int> logical_pages_in_window;
        for (unsigned long long i = base_index; i < base_index + window_size && i < NUMBER_OF_PAGES; ++i) {
            logical_pages_in_window.insert(writing_sequence[i]); // will not add duplicates
        }
        return logical_pages_in_window.size();
    }

    void generateWritingSequence() {
        /* generate a writing sequence according to the desired writing page_dist */
        if (page_dist == UNIFORM) {
            writing_sequence = generateUniformlyDistributedWriteSequence();
        }
        else {
            if (output_file) {
                //dup2(fd_stdout, 1);
                cout << "Should not get here..." << endl;
            }
            cout << "Please enter parameters for Hot/Cold memory simulation." << endl << "Enter the hot page percentage out of all logical pages in memory (0-100): " << endl;
            cin >> user_parameters.hot_pages_percentage;
            if (user_parameters.hot_pages_percentage < 0 || user_parameters.hot_pages_percentage > 100) {
                cerr << "Error! Hot pages percentage must be in 0-100 range. Use --help for more information." << endl;
                exit(-1);
            }
            cout << "Enter the probability for hot pages (0-1): " << endl;
            cin >> user_parameters.hot_pages_probability;
            if (user_parameters.hot_pages_probability < 0 || user_parameters.hot_pages_probability > 1) {
                cerr << "Error! Hot pages probability must be in 0-1 range. Use --help for more information." << endl;
                exit(-1);
            }
            if (output_file)
                freopen(output_file, "a", stdout);
            writing_sequence = generateHotColdWriteSequence(user_parameters.hot_pages_percentage, user_parameters.hot_pages_probability);
        }
    }

    void getUserParams() {
        if (algo != GREEDY) {
            if (window_size_flag == WINDOW_SIZE_ON)
                getWindowSizeFromUser();
            if (window_size_flag == WINDOW_SIZE_OFF)
                user_parameters.window_size = NUMBER_OF_PAGES;
        }
        if (algo == GENERATIONAL)
            getNumOfGenerationsFromUser();
    }

    map<unsigned int, ListItem>* createLocationsMap(unsigned long long base_index, unsigned int window_size) const {
        unsigned int location_list_size = getLocationListSize(base_index, window_size);
        map<unsigned int, ListItem>* locations_list = new map<unsigned int, ListItem>[location_list_size];
        for (unsigned long long i = base_index; i < base_index + window_size && i < NUMBER_OF_PAGES; ++i) {
            auto iterator = locations_list->find(writing_sequence[i]);
            if (iterator == locations_list->end()) {
                locations_list->insert({ writing_sequence[i],ListItem(writing_sequence[i],i) });
            }
            else {
                iterator->second.addLocation(i);
            }
        }
        return locations_list;
    }

    void initializeKBounds() {
        k_bounds = new unsigned long long[user_parameters.number_of_generations - 1];
        if (user_parameters.generational_type == BASIC_GEN ||
            user_parameters.generational_type == GENx_STATIC_BOUND) {
            for (int i = 0; i < user_parameters.number_of_generations - 1; i++) {
                k_bounds[i] = BETA * (i + 1);
            }
        }
    }

    void initializePagesPerGenCounters() {
        physical_writes_per_gen = new unsigned long long[user_parameters.number_of_generations];
        logical_writes_per_gen = new unsigned long long[user_parameters.number_of_generations];
        for (int i = 0; i < user_parameters.number_of_generations; i++) {
            physical_writes_per_gen[i] = 0;
            logical_writes_per_gen[i] = 0;
        }
    }

    void calcKBounds(unsigned long long base_index, unsigned int size) {
        int num_of_gens = user_parameters.number_of_generations;
        vector<int> writing_sequence_ages;
        for (int i = base_index; i < base_index + size; i++) {
            writing_sequence_ages.push_back(pageScore(i) - i);
        }
        auto iter_right = writing_sequence_ages.begin();
        int start_index = 0;
        for (int i = 0; i < num_of_gens - 1; i++) {
            int interval_size = gens_population[i];    
            auto iter_left = iter_right + interval_size;
            nth_element(iter_right, iter_left, writing_sequence_ages.end());
            k_bounds[i] = writing_sequence_ages[start_index + interval_size];
            start_index += interval_size;
            iter_right += interval_size + 2;
        }     
    }

    void runGreedySimulation(Algorithm algo, unsigned long long window_size = 0) {
        if (reach_steady_state) {
            reachSteadyState();
        }
        for (unsigned long long i = 0; i < window_size; i++) {
            ftl->write(data, writing_sequence[i], algo, writing_sequence, i);
        }
        /* After running LOOK_AHEAD/GENERATIONAL algorithm, now we should run
         * GREEDY for the rest of writing sequence */
        for (unsigned long long i = window_size; i < NUMBER_OF_PAGES; i++) {
            ftl->write(data, writing_sequence[i], GREEDY, writing_sequence, i);
        }
    }

    void runSimulation(Algorithm algorithm) {
        switch (algorithm) {
        case GREEDY:
            cout << "Starting Greedy Algorithm simulation..." << endl;
            runGreedySimulation(GREEDY);
            break;
        case GREEDY_LOOKAHEAD:
            cout << "Starting Greedy LookAhead Algorithm simulation..." << endl;
            runGreedySimulation(GREEDY_LOOKAHEAD, user_parameters.window_size);
            break;
        case GENERATIONAL:
            cout << "Starting Generational Algorithm simulation..." << endl;
            runGenerationalSimulation(user_parameters.number_of_generations, user_parameters.window_size);
            break;
        default:
            cerr << "Error in runSimulation" << endl;
            exit(1);
        }
    }

    void printSimulationResults() const {
        int erases = ftl->erases - ftl->erases_steady;
        int logical_page_writes = ftl->logicalPageWrites - ftl->logicalPageWritesSteady;
        int physical_page_writes = ftl->physicalPageWrites - ftl->physicalPageWritesSteady;
        double wa = (double)physical_page_writes / logical_page_writes;
        //double erasure_factor = erases/(NUMBER_OF_PAGES /(double)PAGES_PER_BLOCK);
        cout << "Simulation Results:" << endl;
        if (algo == GENERATIONAL) {
            cout << "Writes per generation:" << endl;
            for (int i = 0; i < user_parameters.number_of_generations; i++) {
                cout << "Generation " << i << " logical writes:\t" <<
                    ((double)logical_writes_per_gen[i] / NUMBER_OF_PAGES) * 100 << endl;
                cout << "Generation " << i << " physical writes:\t" <<
                    ((double)physical_writes_per_gen[i] / physical_page_writes) * 100 << endl;
            }
        }
        cout << "Number of erases: " << erases << ". Write Amplification: " << wa << endl;
    }

    /*void runWritingAssignmentSimulation() {
        if (reach_steady_state) {
            reachSteadyState();
        }
        unsigned long long base_index = 0;
        unsigned int window_size = getWindowSize();
        while (base_index < NUMBER_OF_PAGES) {
            vector<pair<unsigned int, int>> writing_assignment = getWritingAssignment(base_index, window_size);
            for (unsigned long long i = 0; i < writing_assignment.size() && i < NUMBER_OF_PAGES; i++) {
                ftl->writeToBlock(data, writing_assignment[i].first, writing_assignment[i].second);
            }
            base_index += window_size;
            window_size = getWindowSize();
        }
    }*/

    unsigned int getWindowSize() const {
        if (page_dist == UNIFORM) {
            return (PHYSICAL_BLOCK_NUMBER * PAGES_PER_BLOCK) - ftl->windowSizeAux();
        }
        return (PHYSICAL_BLOCK_NUMBER * PAGES_PER_BLOCK) - ftl->getNumberOfValidPages();
    }

    void getWindowSizeFromUser() {
        if (output_file) {
            //dup2(fd_stdout, 1);
            cout << "shouldn't get here.." << endl;
        }
        cout << "Enter Window Size:" << endl;
        cin >> user_parameters.window_size;
        if (user_parameters.window_size > NUMBER_OF_PAGES) {
            cerr << "Error! Window size is bigger than Number Of Pages." << endl;
            printHelp();
            exit(-1);
        }
        if (output_file)
            freopen(output_file, "a", stdout);
        cout << "Window size set successfully to n=" << user_parameters.window_size << "." << endl;
        cout << endl;
    }

    void getNumOfGenerationsFromUser() {
        if (output_file) {
            //dup2(fd_stdout, 1);
            cout << "shouldn't get here.." << endl;
        }
        cout << "Enter number of generations for Generational GC (Enter 0 for heuristic selection):" << endl;
        cin >> user_parameters.number_of_generations; 
        if (output_file)
            freopen(output_file, "a", stdout);
        /*if (user_parameters.number_of_generations > PHYSICAL_BLOCK_NUMBER - LOGICAL_BLOCK_NUMBER) {
            cerr << "Error! number of generations must be at most T-U. Use --help for more information." << endl;
            exit(-1);
        }*/
        if (user_parameters.number_of_generations < 0) {
            cerr << "Error! Window size is a negative number. Use --help for more information." << endl;
            exit(-1);
        }
        cout << "Enter the number of generational algorithm you wish to run:" << endl;
        cout << "1 - Generational GC, BETA is not used" << endl;
        cout << "2 - Generational GCx with static bound, BETA is the static bound" << endl;
        cout << "3 - Generational GCx with Load-Balancing - optimal values assigned, BETA is not used" << endl;
        cout << "4 - Generational GCx with Load-Balancing - manualy values assigned, BETA is interval size" << endl;
        int temp_algo_type;
        cin >> temp_algo_type;
        if (temp_algo_type < 1 || temp_algo_type > 4 ) {
            cerr << "Error! not a generational algorithem type" << endl;
            exit(-1);
        }
        user_parameters.generational_type = GenAlgoType(temp_algo_type - 1);
        cout << "generational algorithm type set to " << user_parameters.generational_type << endl;
        if (user_parameters.number_of_generations == 0) {
            if (user_parameters.generational_type == BASIC_GEN) {
                user_parameters.number_of_generations = ftl->optimized_params.second;
                cout << "Using Overloading factor heuristic to select number of generations..." << endl;
            }
            else {
                double op = (PHYSICAL_BLOCK_NUMBER - LOGICAL_BLOCK_NUMBER) / LOGICAL_BLOCK_NUMBER;
                if (op < 0.1) {
                    user_parameters.number_of_generations = 6;
                }
                else if (op > 0.4 && op < 0.5) {
                    user_parameters.number_of_generations = 9;
                }
                else {
                    user_parameters.number_of_generations = 8;
                }
            }
        }
        if (user_parameters.generational_type == BASIC_GEN) {
            BETA = LOGICAL_BLOCK_NUMBER * PAGES_PER_BLOCK / user_parameters.number_of_generations;
        }
        cout << "Number of generations set to " << user_parameters.number_of_generations << " generations." << endl;
        cout << endl;
    }


    unsigned long long pageScore(unsigned long long page_index, bool page_in_writing_sequence = true, int lpn = NA) const {
        if (page_in_writing_sequence) {
            lpn = writing_sequence[page_index];
        }
        if (logical_pages_location_map->find(lpn) == logical_pages_location_map->end()) {
            return NUMBER_OF_PAGES;
        }
        assert(lpn != NA);
        return logical_pages_location_map->at(lpn).getFirstLocationAfterIndex(page_index);
    }

    void runGenerationalSimulation(int num_of_gens, unsigned long long window_size) {
        if (reach_steady_state) {
            reachSteadyState();
        }

        for (int j = 0; j < num_of_gens; ++j) {
            Block* new_gen_block = nullptr;
            ftl->gen_blocks.insert({ j, new_gen_block });
        }
        if (user_parameters.generational_type == BASIC_GEN) {
            for (unsigned long long i = 0; i < window_size; ++i) {
                int generation = getGeneration(i, num_of_gens);
                physical_writes_per_gen[generation]++;
                logical_writes_per_gen[generation]++;
                ftl->writeGenerational(data, writing_sequence[i], generation, writing_sequence, i);
            }
        }
        else if (user_parameters.generational_type == GENx_STATIC_BOUND) {
            for (unsigned long long i = 0; i < window_size; ++i) {
                while (!gc_pages_to_rewrite.empty()) {
                    int j = 0;
                    while (j < PAGES_PER_BLOCK && gc_pages_to_rewrite.front()[j]) {
                        int generation = getGeneration(i, num_of_gens, false, (gc_pages_to_rewrite.front()[j])->lpn);
                        physical_writes_per_gen[generation]++;
                        ftl->writeGenerational(data, (gc_pages_to_rewrite.front()[j])->lpn, generation, writing_sequence, i, true, &gc_pages_to_rewrite);
                        j++;
                        ftl->rewriten++;
                    }
                    delete[] gc_pages_to_rewrite.front();
                    gc_pages_to_rewrite.pop_front();
                }
                int generation = getGeneration(i, num_of_gens);
                physical_writes_per_gen[generation]++;
                logical_writes_per_gen[generation]++;
                ftl->writeGenerational(data, writing_sequence[i], generation, writing_sequence, i, true, &gc_pages_to_rewrite);
            }
        }
        else {
            for (int n = 0; n < window_size / interval; n++) {
                calcKBounds(n * interval, interval);
                for (unsigned long long i = n * interval; i < n * interval + interval; ++i) {
                    while (!gc_pages_to_rewrite.empty()) {
                        int j = 0;
                        while (j < PAGES_PER_BLOCK && gc_pages_to_rewrite.front()[j]) {
                            int generation = getGeneration(i, num_of_gens, false,
                                (gc_pages_to_rewrite.front()[j])->lpn);
                            physical_writes_per_gen[generation]++;
                            ftl->writeGenerational(data, (gc_pages_to_rewrite.front()[j])->lpn, generation, 
                                writing_sequence, i, true, &gc_pages_to_rewrite);
                            j++;
                        }
                        delete[] gc_pages_to_rewrite.front();
                        gc_pages_to_rewrite.pop_front();
                    }
                    int generation = getGeneration(i, num_of_gens);
                    physical_writes_per_gen[generation]++;
                    logical_writes_per_gen[generation]++;
                    ftl->writeGenerational(data, writing_sequence[i], generation, writing_sequence, i,
                        true, &gc_pages_to_rewrite);
                }
            }
        }
        for(std::map<int,Block*>::iterator it = ftl->gen_blocks.begin(); it!=ftl->gen_blocks.end(); it++){
            /* push generational blocks to freelist */
            if (it->second) {
                assert(it->second->nextFree != NA);
                ftl->freeList.push_back(it->second);
            }
        }
        ftl->gen_blocks.clear();
        for (unsigned long long i = window_size; i < NUMBER_OF_PAGES; i++) {
            ftl->write(data, writing_sequence[i], GREEDY, writing_sequence, i);
        }
    }

    int getGeneration(unsigned long long page_index, int num_of_gens, bool page_in_writing_sequence = true, int lpn = NA) const {
        unsigned long long page_score = pageScore(page_index, page_in_writing_sequence, lpn) - page_index;
        //if (KISS() % 2) {
            for (int i = 0; i < num_of_gens - 1; ++i) {
                if (page_score < k_bounds[i])
                    return i;
            }
            return num_of_gens - 1;
        /* }
        for (int i = num_of_gens - 2; i >= 0; --i) {
            if (page_score >= k_bounds[i])
                return i;
        }
        return 0;*/
    }

};



#endif //FLASHGC_ALGORUNNER_H
