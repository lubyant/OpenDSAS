class Opendsas < Formula
  desc "High-performance Digital Shoreline Analysis System CLI"
  homepage "https://github.com/lubyant/OpenDSAS"
  version "1.4"
  license "MIT"

  # libomp is required at runtime — the binary uses @rpath/libomp.dylib
  # and Homebrew's libomp prefix is on the rpath baked in at build time.
  depends_on "libomp"

  on_arm do
    url "https://github.com/lubyant/OpenDSAS/releases/download/v#{version}/dsas-v#{version}-macos-arm64.tar.gz"
    sha256 "<ARM64_SHA256>"
  end

  def install
    bin.install "dsas"
  end

  test do
    system bin/"dsas", "--version"
  end
end
