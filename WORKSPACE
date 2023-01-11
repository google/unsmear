load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
    ],
    sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
)
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()

http_archive(
    name = "com_google_absl",
    sha256 = "91ac87d30cc6d79f9ab974c51874a704de9c2647c40f6932597329a282217ba8",  # SHARED_ABSL_SHA
    strip_prefix = "abseil-cpp-20220623.1",
    urls = [
        "https://github.com/abseil/abseil-cpp/archive/refs/tags/20220623.1.tar.gz",
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
    sha256 = "22fdaf641b31655d4b2297f9981fa5203b2866f8332d3c6333f6b0107bb320de",
    strip_prefix = "protobuf-21.12",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/refs/tags/v21.12.tar.gz"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()
