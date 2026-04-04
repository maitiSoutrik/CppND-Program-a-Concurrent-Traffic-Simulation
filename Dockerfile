# Docker image for building and testing C++ Concurrent Traffic Simulation
FROM ubuntu:22.04

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libopencv-dev \
    libgtest-dev \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Build Google Test from source (Ubuntu package doesn't include compiled libs)
WORKDIR /usr/src/googletest
RUN cmake . && make && make install

# Set working directory for the project
WORKDIR /app

# Copy source files
COPY src/ ./src/
COPY tests/ ./tests/
COPY data/ ./data/
COPY CMakeLists.txt .

# Build only the tests (not the main simulation which requires GUI)
RUN rm -rf build && mkdir build && cd build && \
    cmake .. && \
    make traffic_simulation_tests -j$(nproc)

# Set entrypoint for running tests
ENTRYPOINT ["./build/traffic_simulation_tests"]
