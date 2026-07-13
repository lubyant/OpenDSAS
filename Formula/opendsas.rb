class Opendsas < Formula
  desc "High-performance Digital Shoreline Analysis System CLI"
  homepage "https://github.com/lubyant/OpenDSAS"
  # `version` is declared before `url` so the url can interpolate it — a
  # single source of truth for the release. (This trips Homebrew's
  # ComponentsOrder audit hint, which wants url first; harmless for a tap.)
  version "1.5"
  url "https://github.com/lubyant/OpenDSAS/releases/download/v#{version}/opendsas-v#{version}-macos-arm64.tar.gz"
  sha256 "4ddb43516fd3e29928897f0d5671c9f89ca24a95edbd7df791636f1e21f63893"
  license "MIT"

  # Track the newest GitHub release so `brew livecheck` / `bump-formula-pr`
  # can flag when url+sha256 need updating for a new version.
  livecheck do
    url :stable
    strategy :github_latest
  end

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
