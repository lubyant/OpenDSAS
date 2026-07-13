class Opendsas < Formula
  desc "High-performance Digital Shoreline Analysis System CLI"
  homepage "https://github.com/lubyant/OpenDSAS"
  url "https://github.com/lubyant/OpenDSAS/releases/download/v1.5/opendsas-v1.5-macos-arm64.tar.gz"
  version "1.5"
  sha256 "4ddb43516fd3e29928897f0d5671c9f89ca24a95edbd7df791636f1e21f63893"
  license "MIT"

  # Only an Apple Silicon build is published.
  depends_on arch: :arm64
  # libomp is required at runtime — the binary uses @rpath/libomp.dylib
  # and Homebrew's libomp prefix is on the rpath baked in at build time.
  depends_on "libomp"

  def install
    bin.install "dsas"
  end

  test do
    system bin/"dsas", "--version"
  end
end
