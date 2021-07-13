load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# bazel-skylib 1.0.3 released 2020.08.27 (https://github.com/bazelbuild/bazel-skylib/releases/tag/1.0.3)
http_archive(
    name = "bazel_skylib",
    sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
    type = "tar.gz",
    urls = [
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
    ],
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
    sha256 = "528927e398f4e290001886894dac17c5c6a2e5548f3fb68004cfb01af901b53a",
    strip_prefix = "protobuf-3.17.3",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.17.3.zip"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()
