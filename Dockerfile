FROM ubuntu:22.04

# Install build tools
RUN apt-get update \
 && apt-get install -y build-essential \
 && rm -rf /var/lib/apt/lists/* \
 && :

# Working directory inside container
WORKDIR /app

# Default command (can be overridden)
CMD ["bash"]
