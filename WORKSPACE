load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# bazel-skylib 1.0.3 released 2020.08.27 (https://github.com/bazelbuild/bazel-skylib/releases/tag/1.0.3)
skylib_version = "1.0.3"

http_archive(
    name = "bazel_skylib",
    sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
    type = "tar.gz",
    url = "https://github.com/bazelbuild/bazel-skylib/releases/download/{}/bazel-skylib-{}.tar.gz".format(skylib_version, skylib_version),
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
    sha256 = "e1853700543f5dfccf05648bb54f16e0add0154a03024334e8b0abd96655f652",
    strip_prefix = "protobuf-909a0f36a10075c4b4bc70fdee2c7e32dd612a72",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/909a0f36a10075c4b4bc70fdee2c7e32dd612a72.zip"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()
