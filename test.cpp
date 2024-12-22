// Copyright (c) 2024 Your Name
// 
// Licensed under the MIT License. See LICENSE file in the project root for details.

#include "unorm.h"

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

int main()
{
    // Define the size of the list (1GB in bytes, divided by the size of a float).
    constexpr size_t sizeInBytes = 1L << 30; // 1GB
    constexpr size_t numFloats = sizeInBytes / sizeof(float);

    // Allocate the list of floats.
    std::vector<float> data(numFloats);
    std::vector<mnorm<32>> data_norm(numFloats);

    // Initialize the list with random float values.
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (size_t i = 0; i < numFloats; ++i) 
    {
        data[i] = dist(rng);
    }

    for (size_t i = 0; i < numFloats; ++i) 
    {
        data_norm[i] = data[i];
    }

    {
        // Perform pairwise multiplication and store results in the first entry of each pair.
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < numFloats; i += 2) {
            if (i + 1 < numFloats) {
                data[i] *= data[i + 1];
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        // Output performance results.
        std::cout << "Floats took " << elapsed.count() << " seconds.\n";
    }
    {
        // Perform pairwise multiplication and store results in the first entry of each pair.
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < numFloats; i += 2) {
            if (i + 1 < numFloats) {
                data_norm[i] *= data_norm[i + 1];
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        // Output performance results.
        std::cout << "Mnorms took " << elapsed.count() << " seconds.\n";
    }

    return 0;
}