# CHANGED: Generic Zephyr development image for all supported targets.
# Optimized for a generic SSH-based development workflow (e.g., with Zed)

# Settings
ARG DEBIAN_VERSION=stable-20241016-slim
ARG PASSWORD="ESP32S3development"
ARG ESP32_MODEL=esp32s3
ARG ESP_SDK_VERSION=v5.4.2
ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll"
ARG VIRTUAL_ENV=/opt/venv

#-------------------------------------------------------------------------------
# Base Image and Dependencies

# Use Debian as the base image
FROM debian:${DEBIAN_VERSION}

# Redeclare arguments after FROM
ARG PASSWORD
ARG ESP32_MODEL
ARG ESP_SDK_VERSION
# REMOVED: TOOLCHAIN_LIST
ARG WGET_ARGS
ARG VIRTUAL_ENV
ARG TARGETARCH

# Set default shell during Docker image build to bash
SHELL ["/bin/bash", "-c"]

# Check if the target architecture is either x86_64 (amd64) or arm64 (aarch64)
RUN if [ "$TARGETARCH" = "amd64" ]; then \
    export HOST_ARCH="x86_64"; \
    elif [ "$TARGETARCH" = "arm64" ]; then \
    export HOST_ARCH="aarch64"; \
    else \
    echo "Unsupported architecture: $TARGETARCH"; \
    exit 1; \
    fi && \
    echo "Architecture $TARGETARCH is supported."

# Set non-interactive frontend for apt-get to skip any user confirmations
ENV DEBIAN_FRONTEND=noninteractive

# Install base packages
# Note: openssh-server is kept as it's essential for the Zed SSH workflow
RUN apt-get -y update && \
    apt-get install --no-install-recommends -y \
    dos2unix \
    bison \
    libssl-dev \
    libffi-dev \
    dfu-util \
    libusb-1.0-0 \
    ca-certificates \
    file \
    locales \
    flex \
    ccache \
    git \
    build-essential \
    cmake \
    ninja-build gperf \
    device-tree-compiler \
    wget \
    curl \
    python3 \
    python3-pip \
    python3-venv \
    xz-utils \
    dos2unix \
    vim \
    nano \
    mc \
    openssh-server \
    clangd

# Set root password
RUN echo "root:${PASSWORD}" | chpasswd

# Set up a Python virtual environment
# ENV VIRTUAL_ENV=${VIRTUAL_ENV}
# RUN python3 -m venv ${VIRTUAL_ENV}
# ENV PATH="${VIRTUAL_ENV}/bin:$PATH"

# Clean up stale packages
RUN apt-get clean -y && \
    apt-get autoremove --purge -y && \
    rm -rf /var/lib/apt/lists/*

# Set up directories
RUN mkdir -p /workspace/

# Set up sshd working directory
RUN mkdir -p /var/run/sshd && \
    chmod 0755 /var/run/sshd

# Allow root login via SSH
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config

# Expose SSH port
EXPOSE 22

#-------------------------------------------------------------------------------
# Install ESP_SDK
RUN mkdir -p ~/esp && \
    cd ~/esp && \
    git clone -b ${ESP_SDK_VERSION} --recursive https://github.com/espressif/esp-idf.git

#-------------------------------------------------------------------------------
# Set-up Tools
RUN cd ~/esp/esp-idf && \
    ./install.sh ${ESP32_MODEL}

#-------------------------------------------------------------------------------
# Optional Settings

RUN sed -i '/^#.*en_PH.UTF-8/s/^#//' /etc/locale.gen && \
    locale-gen en_PH.UTF-8 && \
    update-locale LANG=en_PH.UTF-8 LC_ALL=en_PH.UTF-8

# Use the "dark" theme for Midnight Commander
ENV MC_SKIN=dark

#-------------------------------------------------------------------------------
# Optional Settings
RUN echo "source ~/esp/esp-idf/export.sh" >> /root/.bashrc && \
    echo "export IDF_PATH=~/esp/esp-idf" >> /root/.bashrc

# Set working directory for the user
COPY scripts/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh && \
    dos2unix /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
