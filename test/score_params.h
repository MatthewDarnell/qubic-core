#pragma once

namespace score_params
{
enum ParamType
{
    NR_NEURONS = 0,
    NR_NEIGHBOR_NEURONS,
    DURATIONS,
    NR_OPTIMIZATION_STEPS,
    MAX_PARAM_TYPE
};

static constexpr unsigned long long kDataLength = 256;

// Comment out when we want to reduce the number of running test
static constexpr unsigned long long kSettings[][MAX_PARAM_TYPE] = {
    {512, 256, 100000, 10},
    {856, 520, 120000, 20},
    {1000, 1000, 1000000, 30},
    {2844, 2160, 9000000ULL, 89},
    {3000, 3000, 9000000ULL, 100},
    //{4208, 4016, 10000000ULL, 211},
    //{6000, 6000, 36000000ULL, 457},
    //{6800, 6200, 37000000ULL, 480},
    //{8208, 7000, 40000000ULL, 512},
    //{10000, 10000, 100000000ULL, 620},
};
}
