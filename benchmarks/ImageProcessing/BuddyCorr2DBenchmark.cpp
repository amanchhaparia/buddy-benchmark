//===- BuddyCorr2DBenchmark.cpp -------------------------------------------===//
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//
//
// This file implements the benchmark for Corr2D operation.
//
//===----------------------------------------------------------------------===//

#include "Kernels.h"
#include <benchmark/benchmark.h>
#include <buddy/core/Container.h>
#include <buddy/core/ImageContainer.h>
#include <buddy/dip/dip.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// Declare input image.
Mat inputImageBuddyCorr2D;

// Define the kernel data and size.
float *kernelDataBuddyCorr2D;
int kernelRowsBuddyCorr2D, kernelColsBuddyCorr2D;

// Define the output size.
int outputRowsBuddyCorr2D, outputColsBuddyCorr2D;

// Define sizes of input, kernel, and output.
intptr_t sizesInputBuddyCorr2D[2];
intptr_t sizesKernelBuddyCorr2D[2];
intptr_t sizesOutputBuddyCorr2D[2];

// Declare Boundary Options supported.
enum BoundaryOption { constant_padding, replicate_padding };

// Define Boundary option selected.
BoundaryOption BoundaryType;

void initializeBuddyCorr2D(char **argv) {
  inputImageBuddyCorr2D = imread(argv[1], IMREAD_GRAYSCALE);

  kernelDataBuddyCorr2D = get<0>(kernelMap[argv[2]]);
  kernelRowsBuddyCorr2D = get<1>(kernelMap[argv[2]]);
  kernelColsBuddyCorr2D = get<2>(kernelMap[argv[2]]);

  outputRowsBuddyCorr2D = inputImageBuddyCorr2D.rows;
  outputColsBuddyCorr2D = inputImageBuddyCorr2D.cols;

  sizesInputBuddyCorr2D[0] = inputImageBuddyCorr2D.rows;
  sizesInputBuddyCorr2D[1] = inputImageBuddyCorr2D.cols;

  sizesKernelBuddyCorr2D[0] = kernelRowsBuddyCorr2D;
  sizesKernelBuddyCorr2D[1] = kernelColsBuddyCorr2D;

  sizesOutputBuddyCorr2D[0] = outputRowsBuddyCorr2D;
  sizesOutputBuddyCorr2D[1] = outputColsBuddyCorr2D;

  if (static_cast<string>(argv[3]) == "REPLICATE_PADDING") {
    BoundaryType = replicate_padding;
  } else {
    BoundaryType = constant_padding;
  }
}

static void Buddy_Corr2D_Constant_Padding(benchmark::State &state) {
  // Define the MemRef descriptor for input, kernel, and output.
  Img<float, 2> inputBuddyCorr2D(inputImageBuddyCorr2D);
  MemRef<float, 2> kernelBuddyCorr2D(kernelDataBuddyCorr2D,
                                     sizesKernelBuddyCorr2D);
  MemRef<float, 2> outputBuddyCorr2D(sizesOutputBuddyCorr2D);

  for (auto _ : state) {
    for (int i = 0; i < state.range(0); ++i) {
      // Call the MLIR Corr2D function.
      dip::Corr2D(&inputBuddyCorr2D, &kernelBuddyCorr2D, &outputBuddyCorr2D,
                  1 /* Center X */, 1 /* Center Y */,
                  dip::BOUNDARY_OPTION::CONSTANT_PADDING,
                  0.0f /* Constant Value*/);
    }
  }
}

static void Buddy_Corr2D_Replicate_Padding(benchmark::State &state) {
  // Define the MemRef descriptor for input, kernel, and output.
  Img<float, 2> inputBuddyCorr2D(inputImageBuddyCorr2D);
  MemRef<float, 2> kernelBuddyCorr2D(kernelDataBuddyCorr2D,
                                     sizesKernelBuddyCorr2D);
  MemRef<float, 2> outputBuddyCorr2D(sizesOutputBuddyCorr2D);

  for (auto _ : state) {
    for (int i = 0; i < state.range(0); ++i) {
      // Call the MLIR Corr2D function.
      dip::Corr2D(&inputBuddyCorr2D, &kernelBuddyCorr2D, &outputBuddyCorr2D,
                  1 /* Center X */, 1 /* Center Y */,
                  dip::BOUNDARY_OPTION::REPLICATE_PADDING,
                  0.0f /* Constant Value*/);
    }
  }
}

// Register benchmarking function.
void registerBenchmarkBuddyCorr2D() {
  if (BoundaryType == replicate_padding) {
    BENCHMARK(Buddy_Corr2D_Replicate_Padding)
        ->Arg(1)
        ->Unit(benchmark::kMillisecond);
  } else {
    BENCHMARK(Buddy_Corr2D_Constant_Padding)
        ->Arg(1)
        ->Unit(benchmark::kMillisecond);
  }
}

// Generate result image.
void generateResultBuddyCorr2D(char **argv) {
  // Define the MemRef descriptor for input, kernel, and output.
  Img<float, 2> input(inputImageBuddyCorr2D);
  MemRef<float, 2> kernel(kernelDataBuddyCorr2D, sizesKernelBuddyCorr2D);
  MemRef<float, 2> output(sizesOutputBuddyCorr2D);
  // Run the 2D correlation.
  if (static_cast<string>(argv[3]) == "REPLICATE_PADDING") {
    // Call the MLIR Corr2D function.
    dip::Corr2D(&input, &kernel, &output, 1 /* Center X */, 1 /* Center Y */,
                dip::BOUNDARY_OPTION::REPLICATE_PADDING,
                0.0f /* Constant Value*/);
  } else {
    // Call the MLIR Corr2D function.
    dip::Corr2D(&input, &kernel, &output, 1 /* Center X */, 1 /* Center Y */,
                dip::BOUNDARY_OPTION::CONSTANT_PADDING,
                0.0f /* Constant Value*/);
  }

  // Define a cv::Mat with the output of the correlation.
  Mat outputImage(outputRowsBuddyCorr2D, outputColsBuddyCorr2D, CV_32FC1,
                  output.getData());

  // Choose a PNG compression level
  vector<int> compressionParams;
  compressionParams.push_back(IMWRITE_PNG_COMPRESSION);
  compressionParams.push_back(9);

  // Write output to PNG.
  bool result = false;
  try {
    result = imwrite("ResultBuddyCorr2D.png", outputImage, compressionParams);
  } catch (const cv::Exception &ex) {
    fprintf(stderr, "Exception converting image to PNG format: %s\n",
            ex.what());
  }
  if (result)
    cout << "Saved PNG file." << endl;
  else
    cout << "ERROR: Can't save PNG file." << endl;
}
