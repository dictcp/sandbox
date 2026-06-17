#!/usr/bin/env bash
set -u

say() {
  printf "\n== %s ==\n" "$1"
}

ok() {
  printf "✅ %s\n" "$1"
}

warn() {
  printf "⚠️  %s\n" "$1"
}

fail() {
  printf "❌ %s\n" "$1"
}

cmd_exists() {
  command -v "$1" >/dev/null 2>&1
}

run_test() {
  local desc="$1"
  shift

  if "$@" >/tmp/container-check.out 2>/tmp/container-check.err; then
    ok "$desc"
    sed 's/^/   /' /tmp/container-check.out | head -20
  else
    fail "$desc"
    sed 's/^/   stderr: /' /tmp/container-check.err | head -20
  fi
}

say "basic system"
uname -a
printf "user: %s\n" "$(id)"
printf "shell pid: %s\n" "$$"

if [ -r /etc/os-release ]; then
  . /etc/os-release
  printf "os: %s\n" "${PRETTY_NAME:-unknown}"
fi

say "am I already inside a container?"
if [ -f /.dockerenv ]; then
  warn "Found /.dockerenv"
fi

if grep -qaE 'docker|lxc|kubepods|containerd|libpod' /proc/1/cgroup 2>/dev/null; then
  warn "/proc/1/cgroup suggests container-like environment:"
  cat /proc/1/cgroup
else
  ok "/proc/1/cgroup does not obviously show docker/lxc/k8s"
fi

if cmd_exists systemd-detect-virt; then
  systemd-detect-virt || true
fi

say "kernel user namespace knobs"
if [ -r /proc/sys/kernel/unprivileged_userns_clone ]; then
  v="$(cat /proc/sys/kernel/unprivileged_userns_clone)"
  printf "kernel.unprivileged_userns_clone=%s\n" "$v"
  if [ "$v" = "1" ]; then ok "unprivileged user namespace clone appears enabled"; else warn "unprivileged user namespace clone disabled"; fi
else
  warn "/proc/sys/kernel/unprivileged_userns_clone not present"
fi

if [ -r /proc/sys/user/max_user_namespaces ]; then
  v="$(cat /proc/sys/user/max_user_namespaces)"
  printf "user.max_user_namespaces=%s\n" "$v"
  if [ "$v" -gt 0 ] 2>/dev/null; then ok "user namespaces quota is non-zero"; else warn "user namespace quota is zero"; fi
else
  warn "/proc/sys/user/max_user_namespaces not present"
fi

say "namespace creation tests"

if cmd_exists unshare; then
  run_test "unshare user namespace" unshare -Ur true
  run_test "unshare mount namespace" unshare -m true
  run_test "unshare pid namespace" unshare -p -f true
  run_test "unshare net namespace" unshare -n true
  run_test "unshare user+mount namespace" unshare -Ur -m true
else
  fail "unshare command not found"
fi

say "mount tests"

tmpbase="$(mktemp -d /tmp/container-check.XXXXXX)"
mkdir -p "$tmpbase/proc" "$tmpbase/tmpfs" "$tmpbase/lower" "$tmpbase/upper" "$tmpbase/work" "$tmpbase/merged"
echo "hello" > "$tmpbase/lower/hello.txt"

if mount -t tmpfs tmpfs "$tmpbase/tmpfs" >/tmp/container-check.err 2>&1; then
  ok "mount tmpfs"
  umount "$tmpbase/tmpfs" || true
else
  fail "mount tmpfs"
  sed 's/^/   stderr: /' /tmp/container-check.err | head -20
fi

if mount -t proc proc "$tmpbase/proc" >/tmp/container-check.err 2>&1; then
  ok "mount proc"
  umount "$tmpbase/proc" || true
else
  fail "mount proc"
  sed 's/^/   stderr: /' /tmp/container-check.err | head -20
fi

if grep -qw overlay /proc/filesystems 2>/dev/null; then
  ok "kernel overlayfs listed in /proc/filesystems"
else
  warn "kernel overlayfs not listed in /proc/filesystems"
fi

if mount -t overlay overlay \
  -o lowerdir="$tmpbase/lower",upperdir="$tmpbase/upper",workdir="$tmpbase/work" \
  "$tmpbase/merged" >/tmp/container-check.err 2>&1; then
  ok "mount overlayfs"
  ls -la "$tmpbase/merged" | sed 's/^/   /'
  umount "$tmpbase/merged" || true
else
  fail "mount overlayfs"
  sed 's/^/   stderr: /' /tmp/container-check.err | head -20
fi

if cmd_exists fuse-overlayfs; then
  ok "fuse-overlayfs installed: $(command -v fuse-overlayfs)"
else
  warn "fuse-overlayfs not installed"
fi

if [ -e /dev/fuse ]; then
  ok "/dev/fuse exists"
  ls -l /dev/fuse
else
  warn "/dev/fuse missing"
fi

rm -rf "$tmpbase"

say "cgroup"

if [ -f /sys/fs/cgroup/cgroup.controllers ]; then
  ok "cgroup v2 detected"
  printf "controllers: "
  cat /sys/fs/cgroup/cgroup.controllers
else
  warn "cgroup v2 unified hierarchy not detected"
fi

if [ -w /sys/fs/cgroup ]; then
  ok "/sys/fs/cgroup is writable"
else
  warn "/sys/fs/cgroup is not writable"
fi

if [ -r /proc/self/cgroup ]; then
  printf "/proc/self/cgroup:\n"
  sed 's/^/   /' /proc/self/cgroup
fi

say "security: seccomp"

if grep -q '^Seccomp:' /proc/self/status; then
  grep '^Seccomp' /proc/self/status | sed 's/^/   /'

  seccomp_value="$(awk '/^Seccomp:/ {print $2}' /proc/self/status)"
  case "$seccomp_value" in
    0) ok "seccomp disabled for this process" ;;
    1) warn "seccomp strict mode" ;;
    2) warn "seccomp filter mode active; outer environment may block container syscalls" ;;
    *) warn "unknown seccomp value: $seccomp_value" ;;
  esac
else
  warn "cannot read seccomp status"
fi

say "security: capabilities"

if cmd_exists capsh; then
  capsh --print | sed 's/^/   /' | head -80

  if capsh --print | grep -q 'cap_sys_admin'; then
    ok "cap_sys_admin appears somewhere in capsh output"
  else
    warn "cap_sys_admin not present; mount operations likely blocked"
  fi
else
  warn "capsh not installed. On Debian/Ubuntu: apt install libcap2-bin"
  grep '^Cap' /proc/self/status | sed 's/^/   /'
fi

say "security: AppArmor / SELinux"

if [ -r /proc/self/attr/current ]; then
  aa="$(cat /proc/self/attr/current 2>/dev/null || true)"
  printf "AppArmor current: %s\n" "$aa"
  if echo "$aa" | grep -q unconfined; then
    ok "AppArmor appears unconfined"
  else
    warn "AppArmor profile may restrict operations"
  fi
else
  warn "cannot read /proc/self/attr/current"
fi

if cmd_exists getenforce; then
  printf "SELinux: "
  getenforce || true
else
  warn "getenforce not installed or SELinux not active"
fi

say "/proc /sys /dev sanity"

if mountpoint -q /proc; then
  ok "/proc is mounted"
else
  fail "/proc is not a mountpoint"
fi

if mountpoint -q /sys; then
  ok "/sys is mounted"
else
  warn "/sys is not a mountpoint"
fi

if mountpoint -q /dev; then
  ok "/dev is mounted"
else
  warn "/dev is not a mountpoint"
fi

for d in /dev/null /dev/zero /dev/random /dev/urandom /dev/pts /dev/shm; do
  if [ -e "$d" ]; then
    ok "$d exists"
  else
    warn "$d missing"
  fi
done

say "network namespace / virtual ethernet requirements"

if cmd_exists ip; then
  ip link show | sed 's/^/   /' | head -40

  if ip link add veth-test-a type veth peer name veth-test-b >/tmp/container-check.err 2>&1; then
    ok "can create veth pair"
    ip link del veth-test-a >/dev/null 2>&1 || true
  else
    fail "cannot create veth pair"
    sed 's/^/   stderr: /' /tmp/container-check.err | head -20
  fi
else
  warn "ip command not found. On Debian/Ubuntu: apt install iproute2"
fi

say "Docker / Podman availability"

if cmd_exists docker; then
  ok "docker installed: $(docker --version)"
  docker info 2>/tmp/container-check.err | sed 's/^/   /' | head -80 || {
    fail "docker info failed"
    sed 's/^/   stderr: /' /tmp/container-check.err | head -40
  }
else
  warn "docker not installed"
fi

if cmd_exists podman; then
  ok "podman installed: $(podman --version)"
  podman info 2>/tmp/container-check.err | sed 's/^/   /' | head -80 || {
    fail "podman info failed"
    sed 's/^/   stderr: /' /tmp/container-check.err | head -40
  }
else
  warn "podman not installed"
fi

say "minimal runtime smoke tests"

if cmd_exists docker; then
  docker run --rm hello-world >/tmp/container-check.out 2>/tmp/container-check.err
  if [ $? -eq 0 ]; then
    ok "docker hello-world works"
    sed 's/^/   /' /tmp/container-check.out | head -20
  else
    fail "docker hello-world failed"
    sed 's/^/   stderr: /' /tmp/container-check.err | head -40
  fi
fi

if cmd_exists podman; then
  podman run --rm hello-world >/tmp/container-check.out 2>/tmp/container-check.err
  if [ $? -eq 0 ]; then
    ok "podman hello-world works"
    sed 's/^/   /' /tmp/container-check.out | head -20
  else
    fail "podman hello-world failed"
    sed 's/^/   stderr: /' /tmp/container-check.err | head -40
  fi
fi

say "summary hints"

cat <<'EOF'
Read this roughly as:

- unshare mount namespace fails:
    real Docker/Podman/runc probably cannot work.

- mount proc fails:
    normal containers probably cannot start.

- overlayfs fails but fuse-overlayfs exists:
    rootless/container storage may still work, but only if namespaces/mounts are allowed.

- seccomp filter active:
    outer host may block clone/unshare/mount/bpf/keyctl/etc.

- cap_sys_admin missing:
    direct mount operations will fail unless using userns/rootless-compatible paths.

- cgroup v2 not writable:
    Docker may start but resource limits/systemd/container management may be broken.

- veth creation fails:
    bridge networking will fail; rootless/slirp4netns/pasta may still work.

For your previous error, the most important checks are:
    unshare -m true
    unshare -Ur -m true
    mount -t proc proc /tmp/something
EOF
