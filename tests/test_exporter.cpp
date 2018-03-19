#include "gtest/gtest.h"

#define VERBOSITY_NONE

#include "input/generator_null.hpp"
#include "evt/evtapproach_bm.hpp"
#include "statistical/estimator_mle.hpp"
#include "statistical/test_ks.hpp"
#include "statistical/test_ljung_box.hpp"
#include "aec.hpp"
#include "measures_pool.hpp"
#include "timing.hpp"
#include "utility/exporter.hpp"

using namespace chronovise;

#include <cmath>
#include <iostream>
#include <string>
#include <sstream>

// The fixture for testing class Project1. From google test primer.
class Exporter_Test : public ::testing::Test {
protected:


};

class EXP_C_1 : public chronovise::AbstractExecutionContext<unsigned int, unsigned long> {

public:
    EXP_C_1() : null_input_dist(
        new chronovise::NullInputGenerator
    ) {}

    virtual exit_code_t onSetup() {

        /* ********** PRE-RUN SECTION  ********** */
        this->set_input_source(std::move(null_input_dist));

        /* ********** POST-RUN SECTION ********** */

        this->set_merging_technique(merger_type_e::ENVELOPE);

        std::unique_ptr<EVTApproach<unsigned int, unsigned long>> evt_app(
            new EVTApproach_BM<unsigned int, unsigned long>(10)
        );
        this->set_evt_approach(std::move(evt_app), 0.45);

        std::unique_ptr<Estimator<unsigned int, unsigned long>> evt_est(
            new Estimator_MLE<unsigned int, unsigned long>()
        );
        this->set_evt_estimator(std::move(evt_est));

        return AEC_OK;
    }

    virtual exit_code_t onConfigure() {
        return get_input_iteration() > 0 ? AEC_OK : AEC_CONTINUE;
    }

    virtual exit_code_t onRun() {
        this->add_sample((unsigned int) get_iteration()*2 );
        return AEC_OK;
    }

    virtual exit_code_t onMonitor() {

        if (get_iteration() < 49) {
            return AEC_CONTINUE;
        }

        return AEC_OK;
    }

    virtual exit_code_t onRelease() {
        return AEC_OK;
    }

private:
    chronovise::Timing timing;
    std::unique_ptr<chronovise::NullInputGenerator> null_input_dist;

};


TEST_F(Exporter_Test, sample_output) {


    chronovise::AbstractExecutionContext<unsigned int, unsigned long> *aec = new EXP_C_1();

    aec->run();

    Exporter<unsigned, unsigned long> exp(*aec, false);

    EXPECT_NO_THROW(exp.save_time_samples("test_aec_1.txt"));

    std::ifstream test_file("test_aec_1.txt");

    int i;
    for (i=0; ! test_file.eof(); i++) {
        std::string line;
        std::getline(test_file,line);

        if (line.empty()) {
            i--;
            continue;
        }

        std::stringstream line_stream(line);

        std::string cell;
        std::vector<std::string> cells;
        while(std::getline(line_stream, cell, ',')) {
            cells.push_back(cell);
        }
        continue;

        ASSERT_EQ(cells.size(), 2);

        if (i != 0) {
            EXPECT_EQ(cells[0], "0");
            ASSERT_NO_THROW(stoi(cells[1]));
            EXPECT_EQ((i-1)*2, stoi(cells[1]));
        }
    }

    EXPECT_EQ(50+1, i);

    delete aec;

}
