FROM debian:13-slim

ARG UDOCKER_VERSION=1.3.17

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      ca-certificates \
      curl \
      libfakechroot \
      python3 \
      python-is-python3 \
      tar \
      xz-utils \
 && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash udocker

USER udocker
WORKDIR /home/udocker

RUN curl -fL \
      "https://github.com/indigo-dc/udocker/releases/download/${UDOCKER_VERSION}/udocker-${UDOCKER_VERSION}.tar.gz" \
      -o udocker.tar.gz \
 && tar -xzf udocker.tar.gz \
 && rm udocker.tar.gz \
 && ln -s "/home/udocker/udocker-${UDOCKER_VERSION}/udocker" /home/udocker/udocker

WORKDIR /home/udocker/udocker

RUN ./udocker install

COPY --chown=udocker:udocker entrypoint.sh /home/udocker/entrypoint.sh
RUN chmod +x /home/udocker/entrypoint.sh

ENTRYPOINT ["/home/udocker/entrypoint.sh"]
