FROM --platform=linux/arm64 debian:11

# Install required tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    xz-utils \
    openocd \
    && rm -rf /var/lib/apt/lists/*

# Télécharger et installer ARM GNU Toolchain pour ARM64
RUN wget https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz \
    && tar -xf arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz \
    && mv arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi /opt/

WORKDIR /workspace

COPY . .

ENV PATH="/opt/arm-gnu-toolchain-13.3.rel11-x86_64-arm-none-eabi/bin:${PATH}"

ENTRYPOINT [""]
