from ubuntu:24.04 as builder

run --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y \
    build-essential \
    g++-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    llvm \
    clang \
    libc++-dev \
    libc++abi-dev

run mkdir /result
run --mount=type=bind,target=/build clang++ -O3 -std=c++23 -target aarch64-pc-linux-gnu -march=armv8.4-a+sve -fuse-ld=/usr/bin/aarch64-linux-gnu-ld -o /result/main /build/mmul.cpp

from --platform=linux/arm64 ubuntu:24.04
env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu/
copy --from=builder /result/main /main
entrypoint ["/main"]
