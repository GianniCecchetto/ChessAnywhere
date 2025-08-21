FROM --platform=linux/arm64 debian:11

# Install required tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    xz-utils \
    openocd \
    && rm -rf /var/lib/apt/lists/*

# Download and install ARM GNU Toolchain 13.3 (x86_64 host)
RUN curl -LO https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz \
    && tar -xf arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz \
    && mv arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi /opt/arm-toolchain \
    && ln -s /opt/arm-toolchain/bin/* /usr/local/bin/ \
    && rm arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz

WORKDIR /workspace

COPY . .

ENTRYPOINT [""]
