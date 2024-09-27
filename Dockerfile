FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    wget cmake vim \
    build-essential \
    libace-dev \
    && apt-get clean

WORKDIR /app
