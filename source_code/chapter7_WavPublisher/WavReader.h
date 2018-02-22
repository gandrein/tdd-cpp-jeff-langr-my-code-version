/***
 * Excerpted from "Modern C++ Programming with Test-Driven Development",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material,
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose.
 * Visit http://www.pragmaticprogrammer.com/titles/lotdd for more book information.
***/
#ifndef WavReader_h
#define WavReader_h

#include <boost/filesystem.hpp>
#include <fstream>
#include <string>

#include "WavDescriptor.h"
#include "rlog/RLogChannel.h"
#include "rlog/StdioNode.h"

bool HasExtension(const std::string& text, const std::string& substring);

namespace wav_reader {

struct FormatSubchunkHeader;
struct FormatSubchunk {
  uint16_t format_tag;
  uint16_t channels;
  uint32_t samples_per_second;
  uint32_t average_bytes_per_second;
  uint16_t block_align;
  uint16_t bits_per_sample;
};

struct DataChunk {
  uint32_t length;
};

class WavReader {
 public:
  WavReader(const std::string& source, const std::string& dest);
  virtual ~WavReader();
  void Open(const std::string& name, bool trace);
  void List(const boost::filesystem::path& dir, const std::string& filename,
            std::vector<boost::filesystem::path>& found) const;
  void ListAll() const;
  void PublishSnippets();

  uint32_t DataLength(const uint32_t bytes_per_sample, const uint32_t samples, const uint32_t channels) const;

  void WriteWavSnippet(const std::string& name, std::ostream& out, FormatSubchunk& format_subchunk,
                       DataChunk& wav_chunk, char* data);

  void WriteSamples(std::ostream* out, char* data, const uint32_t starting_sample, const uint32_t samples_to_write,
                    const uint32_t bytes_per_sample, const u_int32_t channels = 1);

  uint32_t total_seconds_to_write;
 private:
  void SeekToEndOfHeader(std::ifstream& file, int headerLength);
  void FileReadChunk(std::ifstream& file, DataChunk& data_chunk);

  void ReadAndWriteHeaders(const std::string& name, std::ifstream& file, std::ostream& out,
                           FormatSubchunk& format_subchunk, FormatSubchunkHeader& format_subchunk_header);

  char* FileReadData(std::ifstream& file, const uint32_t length);

  std::string ToString(int8_t* c, unsigned int size);

  rlog::StdioNode log{STDERR_FILENO};
  WavDescriptor* descriptor_;
  rlog::RLogChannel* rlog_channel_;
  std::string source_;
  std::string dest_;
};
}

#endif
