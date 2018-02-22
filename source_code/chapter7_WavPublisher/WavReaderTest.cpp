/***
 * Excerpted from "Modern C++ Programming with Test-Driven Development",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material,
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose.
 * Visit http://www.pragmaticprogrammer.com/titles/lotdd for more book information.
***/
#include "WavReader.h"

#include <iostream>
#include <sstream>
#include <string>

#include "gmock/gmock.h"

using namespace testing;
using namespace wav_reader;

class WavReader_WriteSamples : public ::testing::Test {
 public:
  WavReader reader{".", "."};
  std::ostringstream out;
};

TEST_F(WavReader_WriteSamples, WritesSingleSample) {
  char data[] = "abcd";
  uint32_t bytes_per_sample{1};
  uint32_t starting_sample{0};
  uint32_t samples_to_write{1};

  reader.WriteSamples(&out, data, starting_sample, samples_to_write, bytes_per_sample);

  ASSERT_EQ(out.str(), "a");
}

TEST_F(WavReader_WriteSamples, WritesMultibyteSampleFromMiddle) {
  char data[]{"0123456789ABCDEFG"};  // just keeping this type of initialization for example purposes
  uint32_t bytes_per_sample{2};
  uint32_t starting_sample{4};
  uint32_t samples_to_write{3};

  reader.WriteSamples(&out, data, starting_sample, samples_to_write, bytes_per_sample);

  ASSERT_EQ("89ABCD", out.str());
}

TEST_F(WavReader_WriteSamples, IncorporatesChannelCount) {
  char data[]{"0123456789ABCDEFG"};
  uint32_t bytes_per_sample{2};
  uint32_t starting_sample{0};
  uint32_t samples_to_write{2};
  uint32_t channels{2};

  reader.WriteSamples(&out, data, starting_sample, samples_to_write, bytes_per_sample, channels);

  ASSERT_EQ("01234567", out.str());
}

class WavReader_DataLength : public ::testing::Test {
 public:
  WavReader reader{".", "."};
};

TEST_F(WavReader_DataLength, IsProductOfChannels_bytes_per_sample_and_Samples) {
  uint32_t bytes_per_sample{2};
  uint32_t samples{5};
  uint32_t channels{4};

  uint32_t length = reader.DataLength(bytes_per_sample, samples, channels);

  ASSERT_EQ(2 * 5 * 4, length);
}

class WavReader_WriteSnippet : public ::testing::Test {
 public:
  WavReader reader{"", ""};
  std::istringstream input{""};
  std::ostringstream output;
  FormatSubchunk format_subchunk;
  DataChunk data_chunk;
  uint32_t num_bytes_in_chunk = 8;  // number of bytes
  uint32_t number_of_bits_in_two_bytes{2 * 8};
  char* data;

  void SetUp() override {
    data = new char[4];
    data_chunk.length = num_bytes_in_chunk;
    format_subchunk.bits_per_sample = static_cast<unsigned short>(number_of_bits_in_two_bytes);
    format_subchunk.samples_per_second = 1;
  }

  void TearDown() override { delete[] data; }
};

TEST_F(WavReader_WriteSnippet, UpdatesTotalSeconds) {
  uint32_t num_samples_per_second =
      num_bytes_in_chunk / (number_of_bits_in_two_bytes / 8 / format_subchunk.samples_per_second);

  reader.WriteWavSnippet("any", output, format_subchunk, data_chunk, data);

  ASSERT_EQ(num_samples_per_second, reader.total_seconds_to_write);
}
