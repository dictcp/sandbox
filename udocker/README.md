# udocker

Quick sandbox for running [udocker](https://github.com/indigo-dc/udocker) inside a Debian 12 container.

## Quick host check

If you only want to see whether the current machine is likely to support container-style features, fetch and run `container-check.sh` directly:

```bash
curl -fsSL https://raw.githubusercontent.com/dictcp/sandbox/master/udocker/container-check.sh | sudo bash
```

`sudo` is recommended because checks like `mount`, namespace creation, veth creation, and capability inspection can fail or give incomplete results as an unprivileged user. If you want a non-root pass first, replace `sudo bash` with `bash`.

## What is in this folder

- `Dockerfile`: builds a Debian 12 based image, installs Python and `libfakechroot`, downloads `udocker`, and runs `./udocker install` for the unprivileged `udocker` user.
- `entrypoint.sh`: pulls a container image on demand, creates a temporary udocker container, configures `P1` execution mode, and then runs either the image default command or the command you pass in.
- `container-check.sh`: host-side diagnostic script to test whether namespaces, mounts, overlayfs, cgroups, seccomp, and related pieces are available.

## Build

Build the image from this folder:

```bash
docker build -t local/udocker-test .
```

If you want to pin a different udocker version:

```bash
docker build --build-arg UDOCKER_VERSION=1.3.17 -t local/udocker-test .
```

## Use

Run the default image test:

```bash
docker run --rm local/udocker-test
```

That starts the wrapper with the default image `busybox:latest`.

Run a different image:

```bash
docker run --rm local/udocker-test alpine:latest
```

Run a command inside the pulled image:

```bash
docker run --rm local/udocker-test alpine:latest /bin/sh -c 'uname -a && id && cat /etc/os-release'
```

Run with `bash -c` or `sh -c` explicitly:

```bash
docker run --rm local/udocker-test debian:12-slim bash -c 'echo hello from udocker'
```

## Notes

- This image is mainly useful for experimenting with `udocker`, not for production container hosting.
- The entrypoint creates a fresh temporary udocker container each run, so it behaves more like a disposable test harness than a long-lived environment.
- Whether nested container execution works depends heavily on the outer host. If runs fail, start with `container-check.sh`.
