load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# bazel-skylib 0.8.0 released 2019.03.20 (https://github.com/bazelbuild/bazel-skylib/releases/tag/0.8.0)
skylib_version = "0.8.0"

http_archive(
    name = "bazel_skylib",
    sha256 = "2ef429f5d7ce7111263289644d233707dba35e39696377ebab8b0bc701f7818e",
    type = "tar.gz",
    url = "https://github.com/bazelbuild/bazel-skylib/releases/download/{}/bazel-skylib.{}.tar.gz".format(skylib_version, skylib_version),
)

http_archive(
    name = "com_google_absl",
    sha256 = "cdbfcf4e5d2f61273325a90392ffc224469daf0800e5753ce48032db06d250ee",
    strip_prefix = "abseil-cpp-8f11724067248acc330b4d1f12f0c76d03f2cfb1",
    urls = ["https://github.com/abseil/abseil-cpp/archive/8f11724067248acc330b4d1f12f0c76d03f2cfb1.zip"],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "74c86982f8f4c706b2f4efe05461199fe7d5d6c1602fc0f3223ac3de8dbf10a2",
    strip_prefix = "googletest-d7003576dd133856432e2e07340f45926242cc3a",
    urls = ["https://github.com/google/googletest/archive/d7003576dd133856432e2e07340f45926242cc3a.zip"],
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "314323f52af1fcf73db2018184a660ce0ff9ad72bb674a6d5ac465e089670966",
    strip_prefix = "protobuf-f2a919f58f12574ac04ca79e6b84577dec6f2b43",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/f2a919f58f12574ac04ca79e6b84577dec6f2b43.zip"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()
