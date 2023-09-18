FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libjson-c-dev \
    libc6-dev \
    dnsutils \
    iproute2

COPY . /app
WORKDIR /app

RUN make build

CMD ["make", "run"]
