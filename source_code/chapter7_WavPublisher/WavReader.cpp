/***
 * Excerpted from "Modern C++ Programming with Test-Driven Development",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material,
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose.
 * Visit http://www.pragmaticprogrammer.com/titles/lotdd for more book information.
***/
#include "WavReader.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "rlog/StdioNode.h"
#include "rlog/rlog.h"

#include "Snippet.h"
#include "WavDescriptor.h"

namespace wav_reader {

using namespace std;

bool HasExtension(const string& filename, const string& s) {
  string ext{"." + s};
  if (ext.length() > filename.length()) return false;
  return 0 == filename.compare(filename.length() - ext.length(), ext.length(), ext);
}

struct RiffHeader {
  int8_t id[4];
  uint32_t length;
  int8_t format[4];
};

struct FormatSubchunkHeader {
  int8_t id[4];
  uint32_t subchunk_size;
};

struct FactOrData {
  int8_t tag[4];
};

struct FactChunk {
  uint32_t chunkSize;
  uint32_t samplesPerChannel;
};

WavReader::WavReader(const std::string& source, const std::string& dest, std::shared_ptr<WavDescriptor> descriptor)
    : source_(source), dest_(dest), descriptor_(descriptor), file_util_(nullptr) {
  if (!descriptor_) {
    descriptor_ = std::make_shared<WavDescriptor>(dest);
  }

  rlog_channel_ = DEF_CHANNEL("info/wav", rlog::Log_Debug);
  log.subscribeTo((rlog::RLogNode*)RLOG_CHANNEL("info/wav"));
  rLog(rlog_channel_, "reading from %s writing to %s", source.c_str(), dest.c_str());
}

WavReader::~WavReader() {
  descriptor_.reset();
  delete rlog_channel_;
}

void WavReader::PublishSnippets() {
  boost::filesystem::directory_iterator itEnd;
  for (boost::filesystem::directory_iterator it(source_); it != itEnd; ++it)
    if (!boost::filesystem::is_directory(it->status()) &&
        //  it->path().filename().string()=="husten.wav" &&
        HasExtension(it->path().filename().string(), "wav"))
      Open(it->path().filename().string(), false);
}

string WavReader::ToString(int8_t* bytes, unsigned int size) { return string{(char*)bytes, size}; }

void WavReader::Open(const std::string& name, bool trace) {
  rLog(rlog_channel_, "opening %s", name.c_str());

  ifstream file{name, ios::in | ios::binary};
  if (!file.is_open()) {
    rLog(rlog_channel_, "unable to read %s", name.c_str());
    return;
  }
  ofstream out{dest_ + "/" + name, ios::out | ios::binary};

  FormatSubchunkHeader format_subchunk_header;
  FormatSubchunk format_subchunk;
  ReadAndWriteHeaders(name, file, out, format_subchunk, format_subchunk_header);

  DataChunk wav_chunk;
  FileReadChunk(file, wav_chunk);

  char* data = FileReadData(file, wav_chunk.length);  // memory leak (the pointer inside the call is never released)

  Snippet snippet(file_util_, descriptor_, dest_, rlog_channel_);
  snippet.WriteWavSnippet(name, out, format_subchunk, wav_chunk, data);
}

void WavReader::FileReadChunk(ifstream& file, DataChunk& data_chunk) {
  file.read(reinterpret_cast<char*>(&data_chunk), sizeof(DataChunk));
  rLog(rlog_channel_, "data length = %u", data_chunk.length);
}

void WavReader::SelectFileUtility(std::shared_ptr<FileUtil> file_util) { file_util_ = file_util; }

void WavReader::ReadAndWriteHeaders(const std::string& name, std::ifstream& file, std::ostream& out,
                                    FormatSubchunk& format_subchunk, FormatSubchunkHeader& format_subchunk_header) {
  RiffHeader header;
  file.read(reinterpret_cast<char*>(&header), sizeof(RiffHeader));

  if (ToString(header.id, 4) != "RIFF") {
    rLog(rlog_channel_, "ERROR: %s is not a RIFF file", name.c_str());
    return;
  }
  if (ToString(header.format, 4) != "WAVE") {
    rLog(rlog_channel_, "ERROR: %s is not a wav file: %s", name.c_str(), ToString(header.format, 4).c_str());
    return;
  }
  out.write(reinterpret_cast<char*>(&header), sizeof(RiffHeader));

  file.read(reinterpret_cast<char*>(&format_subchunk_header), sizeof(FormatSubchunkHeader));

  if (ToString(format_subchunk_header.id, 4) != "fmt ") {
    rLog(rlog_channel_, "ERROR: %s expecting 'fmt' for subchunk header; got '%s'", name.c_str(),
         ToString(format_subchunk_header.id, 4).c_str());
    return;
  }

  out.write(reinterpret_cast<char*>(&format_subchunk_header), sizeof(FormatSubchunkHeader));

  file.read(reinterpret_cast<char*>(&format_subchunk), sizeof(FormatSubchunk));

  out.write(reinterpret_cast<char*>(&format_subchunk), sizeof(FormatSubchunk));

  rLog(rlog_channel_, "format tag: %u", format_subchunk.format_tag);  // show as hex?
  rLog(rlog_channel_, "samples per second: %u", format_subchunk.samples_per_second);
  rLog(rlog_channel_, "channels: %u", format_subchunk.channels);
  rLog(rlog_channel_, "bits per sample: %u", format_subchunk.bits_per_sample);

  auto bytes = format_subchunk_header.subchunk_size - sizeof(FormatSubchunk);

  rLog(rlog_channel_, "riff header size = %u", sizeof(RiffHeader));
  rLog(rlog_channel_, "subchunk header size = %u", sizeof(FormatSubchunkHeader));
  rLog(rlog_channel_, "subchunk size = %u", format_subchunk_header.subchunk_size);

  auto additionalBytes = new char[bytes];
  file.read(additionalBytes, bytes);
  out.write(additionalBytes, bytes);

  FactOrData factOrData;
  file.read(reinterpret_cast<char*>(&factOrData), sizeof(FactOrData));
  out.write(reinterpret_cast<char*>(&factOrData), sizeof(FactOrData));

  if (ToString(factOrData.tag, 4) == "fact") {
    FactChunk factChunk;
    file.read(reinterpret_cast<char*>(&factChunk), sizeof(FactChunk));
    out.write(reinterpret_cast<char*>(&factChunk), sizeof(FactChunk));

    file.read(reinterpret_cast<char*>(&factOrData), sizeof(FactOrData));
    out.write(reinterpret_cast<char*>(&factOrData), sizeof(FactOrData));

    rLog(rlog_channel_, "samples per channel: %u", factChunk.samplesPerChannel);
  }

  if (ToString(factOrData.tag, 4) != "data") {
    string tag{ToString(factOrData.tag, 4)};
    rLog(rlog_channel_, "%s ERROR: unknown tag>%s<", name.c_str(), tag.c_str());
    return;
  }
}

char* WavReader::FileReadData(std::ifstream& file, const uint32_t length) {
  auto data = new char[length];
  file.read(data, length);
  return data;
  //  file.close(); // no need to close as istreams are RAII
}

void WavReader::SeekToEndOfHeader(ifstream& file, int subchunk_size) {
  auto bytes = subchunk_size - sizeof(FormatSubchunk) + 1;
  file.seekg(bytes, ios_base::cur);
}

}  // namespace
