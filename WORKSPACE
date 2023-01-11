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
    sha256 = "dcf71b9cba8dc0ca9940c4b316a0c796be8fab42b070bb6b7cab62b48f0e66c4",  # SHARED_ABSL_SHA
    strip_prefix = "abseil-cpp-20211102.0",
    urls = [
        "https://github.com/abseil/abseil-cpp/archive/refs/tags/20211102.0.tar.gz",
    ],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "1c805208d019aabb8be3cddbc6098be8815ee5cf0a7baf526102528fd624c422",
    strip_prefix = "googletest-934542165899c786cb5d8a710529c37184730183",
    urls = ["https://github.com/google/googletest/archive/934542165899c786cb5d8a710529c37184730183.zip"],
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "528927e398f4e290001886894dac17c5c6a2e5548f3fb68004cfb01af901b53a",
    strip_prefix = "protobuf-3.17.3",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.17.3.zip"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()
