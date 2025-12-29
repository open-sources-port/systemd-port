class Libsystemd < Formula
  desc "Port of systemd libsystemd components to macOS"
  homepage "https://github.com/open-sources-port/systemd-port"
  url "https://github.com/open-sources-port/systemd-port/archive/refs/tags/v0.1.0.tar.gz"
  sha256 "PUT_REAL_SHA256_HERE"
  license "LGPL-2.1-or-later"

  depends_on :macos

  depends_on "coreutils"
  depends_on "libgcrypt"
  depends_on "libxcrypt"
  depends_on "ccrypt"
  depends_on "gettext"
  depends_on "util-linux" # provides libmount
  depends_on "meson"
  depends_on "ninja"
  depends_on "pkg-config"
  depends_on "python@3.12"
  depends_on "jinja2-cli"

  def install
    # Use Homebrew compiler toolchain
    ENV["CC"]  = ENV.cc
    ENV["CXX"] = ENV.cxx

    # Provide GNU realpath without global symlinks
    ENV.prepend_path "PATH", Formula["coreutils"].libexec/"gnubin"

    cd "macos-homebrew" do
      system "meson", "setup", "build",
             "--buildtype=debugoptimized",
             "--prefix=#{prefix}",
             "--sysconfdir=#{etc}",
             "--localstatedir=#{var}",
             "-Ddefault-hierarchy=unified",
             "-Dselinux=false",
             "-Dapparmor=false",
             "-Dima=false",
             "-Dsmack=false",
             "-Dpolkit=false",
             "-Dlibaudit=false",
             "-Dresolve=false",
             "-Dtimesyncd=false",
             "-Dmachined=false",
             "-Dlogind=false",
             "-Dnetworkd=false",
             "-Dhomed=false",
             "-Dfirstboot=false",
             "-Dldconfig=false"

      system "meson", "compile", "-C", "build"
      system "meson", "install", "-C", "build"
    end
  end

  test do
    # Ensure library directory exists after install
    assert_predicate lib, :exist?
  end
end
