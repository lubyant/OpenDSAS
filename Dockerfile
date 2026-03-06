# ── Stage 1: build a static GDAL on Debian Bullseye (glibc 2.31) ─────────────
# Bullseye is used so the binary's glibc floor matches Ubuntu 20.04 (Focal).
FROM debian:bullseye AS gdal-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ninja-build pkg-config ca-certificates \
    libcurl4-openssl-dev libgeos-dev libproj-dev \
    libtiff-dev libjpeg-dev libsqlite3-dev zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /gdal
RUN git clone --depth 1 --branch v3.8.5 https://github.com/OSGeo/gdal.git .

RUN cmake -B build -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DGDAL_BUILD_TOOLS=OFF \
    -DGDAL_USE_EXTERNAL_LIBS=ON \
    -DGDAL_BUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=/opt/gdal \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# ── Stage 2: build the app ────────────────────────────────────────────────────
FROM debian:bullseye AS app-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git pkg-config ca-certificates \
    libcurl4-openssl-dev libgeos-dev libproj-dev \
    libtiff-dev libjpeg-dev libsqlite3-dev zlib1g-dev \
    libboost-all-dev libomp-dev \
    && rm -rf /var/lib/apt/lists/*

COPY --from=gdal-builder /opt/gdal /opt/gdal

WORKDIR /app
COPY . .

RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_PREFIX_PATH=/opt/gdal \
    && cmake --build build -j$(nproc) \
    && cmake --install build --prefix /usr/local

# ── Stage 3: minimal runtime image ───────────────────────────────────────────
FROM debian:bullseye-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libgeos-c1 libproj19 libcurl4 libtiff5 libsqlite3-0 libgomp1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=app-builder /usr/local/bin/dsas /usr/local/bin/dsas

ENTRYPOINT ["dsas"]
