FROM debian:bullseye

# Install build tools and GDAL dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    libgeos-dev \
    libproj-dev \
    libtiff-dev \
    libjpeg-dev \
    sqlite3 \
    zlib1g-dev \
    pkg-config \
    ninja-build \
    libboost-all-dev \
    && apt-get clean

# Set workdir
WORKDIR /gdal

# Clone and build GDAL from source
RUN git clone --branch v3.8.5 https://github.com/OSGeo/gdal.git .

# Configure and build GDAL with CMake
RUN cmake -B build -S . -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DGDAL_BUILD_TOOLS=OFF \
    -DGDAL_USE_EXTERNAL_LIBS=ON \
    -DGDAL_BUILD_TESTING=OFF

RUN cmake --build build -j$(nproc)
RUN cmake --install build --prefix /usr/local

WORKDIR /app
COPY . .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)
