// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Simple tool to convert the text proto leap_table.pb to other formats.

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"
#include "unsmear/leap_table.pb.h"
#include "unsmear/unsmear.h"

namespace {

constexpr char kUsage[] = "Usage: leap_table_tool FILENAME\n";

enum class Format { kProto, kTextProto, kJson, kDebug };

}  // namespace

ABSL_FLAG(Format, input, Format::kTextProto, "input format");
ABSL_FLAG(Format, output, Format::kProto, "output format");

namespace {

bool AbslParseFlag(absl::string_view text, Format* format, std::string* error) {
  if (text == "proto") {
    *format = Format::kProto;
    return true;
  }
  if (text == "textproto") {
    *format = Format::kTextProto;
    return true;
  }
  if (text == "json") {
    *format = Format::kJson;
    return true;
  }
  if (text == "debug") {
    *format = Format::kDebug;
    return true;
  }
  *error = "unknown format; must be proto, textproto, json, or debug";
  return false;
}

std::string AbslUnparseFlag(Format format) {
  switch (format) {
    case Format::kProto:
      return "proto";
    case Format::kTextProto:
      return "textproto";
    case Format::kJson:
      return "json";
    case Format::kDebug:
      return "debug";
  }
  return absl::StrCat(format);
}

bool OutputProto(const unsmear::LeapTableProto& pb) {
  return pb.SerializeToFileDescriptor(STDOUT_FILENO);
}

bool OutputTextProto(const unsmear::LeapTableProto& pb) {
  auto stream =
      std::make_unique<google::protobuf::io::FileOutputStream>(STDOUT_FILENO);
  if (!google::protobuf::TextFormat::Print(pb, stream.get())) {
    return false;
  }
  return stream->Close();
}

bool OutputJson(const unsmear::LeapTableProto& pb) {
  std::string s;
  google::protobuf::util::JsonOptions opts;
  if (!google::protobuf::util::MessageToJsonString(pb, &s, opts).ok()) {
    return false;
  }
  std::cout << s;
  return true;
}

bool OutputDebug(const unsmear::LeapTableProto& pb) {
  auto lt = unsmear::NewLeapTableFromProto(pb);
  if (lt == nullptr) {
    LOG(ERROR) << "Failed to construct leap table from proto";
    return false;
  }
  std::cout << lt->DebugString();
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<char*> args = absl::ParseCommandLine(argc, argv);
  absl::InitializeLog();

  if (args.size() != 2) {
    LOG(QFATAL) << kUsage;
  }
  const auto& filename = args[1];

  unsmear::LeapTableProto pb;
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    PLOG(FATAL) << absl::StrCat("Couldn't open ", filename);
  }
  switch (absl::GetFlag(FLAGS_input)) {
    case Format::kProto:
      CHECK(pb.ParseFromFileDescriptor(fd))
          << absl::StrCat("Couldn't parse proto from ", filename);
      break;
    case Format::kTextProto: {
      google::protobuf::io::FileInputStream stream(fd);
      CHECK(google::protobuf::TextFormat::Parse(&stream, &pb))
          << absl::StrCat("Couldn't parse text proto from ", filename);
      break;
    }
    case Format::kJson:
    case Format::kDebug:
      LOG(QFATAL) << "Unsupported --input";
  }

  switch (absl::GetFlag(FLAGS_output)) {
    case Format::kProto:
      CHECK(OutputProto(pb));
      break;
    case Format::kTextProto:
      CHECK(OutputTextProto(pb));
      break;
    case Format::kJson:
      CHECK(OutputJson(pb));
      break;
    case Format::kDebug:
      CHECK(OutputDebug(pb));
      break;
  }
}
