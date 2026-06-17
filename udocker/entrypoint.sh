#!/bin/sh
set -eu

IMAGE="${1:-busybox:latest}"
if [ "$#" -gt 0 ]; then
  shift
fi

CONTAINER_NAME="inner-$$"

cd /home/udocker/udocker

if ! ./udocker inspect -p "$IMAGE" >/dev/null 2>&1; then
  ./udocker pull "$IMAGE"
fi
./udocker create --name="$CONTAINER_NAME" "$IMAGE"
./udocker setup --execmode=P1 --force "$CONTAINER_NAME"

if [ "$#" -eq 0 ]; then
  exec ./udocker run "$CONTAINER_NAME"
else
  case "$1" in
    sh|bash)
      if [ "${2:-}" = "-c" ] && [ "$#" -ge 3 ]; then
        shell_cmd="/bin/$1"
        shell_script=$3
        exec ./udocker run --entrypoint="$shell_cmd" "$CONTAINER_NAME" -c "$shell_script"
      fi
      if [ -x "/bin/$1" ]; then
        shell_cmd="/bin/$1"
        shift
        set -- "$shell_cmd" "$@"
      fi
      ;;
  esac
  exec ./udocker run "$CONTAINER_NAME" "$@"
fi
