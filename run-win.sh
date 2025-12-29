#!/bin/bash
set -euo pipefail # [Safety] Exit immediately if a command exits with a non-zero status.
# ------------------------------------------------------------------
# [Description]
# This script manages the lifecycle of the 'c-dev' Docker image
# based on user input. It demonstrates using a case statement to
# handle cleaning (removal) and execution logic.
# ------------------------------------------------------------------
case "$(uname -s)" in
    Linux*)                 ROOT=$(pwd);;
    Darwin*)                ROOT=$(pwd);;
    CYGWIN*|MINGW*|MSYS*)   ROOT=$(pwd -W);;
    *)                      echo "Unknown OS. Exiting." && exit 1;;
esac
IMAGE_NAME="c-dev"
# ------------------------------------------------------------------
# Function: clean_image
# Description: 
#   1. Identifies containers using the image (running or stopped).
#   2. Force removes those containers.
#   3. Removes the Docker image.
# ------------------------------------------------------------------
clean_image() {
    local containers
    containers=$(docker ps -aq --filter "ancestor=$IMAGE_NAME")             # 'docker ps -aq' lists IDs. Filter by ancestor image
    if [[ -n "$containers" ]]; then                                         # Remove containers if found                                   
        echo "Found containers using '$IMAGE_NAME'. Forcing removal..."
        docker rm -f $containers
    fi    
    if [[ "$(docker images -q "$IMAGE_NAME" 2> /dev/null)" != "" ]]; then   # Remove the image
        docker rmi $(docker images -q "$IMAGE_NAME")
        echo "Removed existing Docker image '$IMAGE_NAME'."
    else
        echo "No existing Docker image '$IMAGE_NAME' found."
    fi
}
# ------------------------------------------------------------------
# Function: build_and_run
# Description: Implements the build (if missing) and run logic
#              provided in your snippet.
# ------------------------------------------------------------------
build_and_run() {
    echo "Preparing to run..."

    # Build the Docker image (not needed if already built), if built already, skip.
    if [[ "$(docker images -q "$IMAGE_NAME" 2> /dev/null)" == "" ]]; then
        echo "Image '$IMAGE_NAME' not found. Building..."
        docker build -t "$IMAGE_NAME" .
    else
        echo "Image '$IMAGE_NAME' found. Skipping build."
    fi

    # Run an interactive shell in the Docker container with the project directory mounted
    # [Note] Using --rm to clean up the container (not the image) after exit
    echo "Starting container..."
    # docker run --rm -v "$ROOT:/app" "$IMAGE_NAME" make
    docker run --rm -it -v "$ROOT:/app" "$IMAGE_NAME"
}







# ------------------------------------------------------------------
# Main Logic: Switch-Case
# ------------------------------------------------------------------
echo "Please select an operation mode:"
echo "1. Run without clean"
echo "2. Run after clean"
echo "3. Exit after clean"
read -p "Enter choice [1-3]: " choice

case "$choice" in
    1) build_and_run;;
    2) clean_image && build_and_run;;
    3) clean_image && exit 0;;
    *) echo "Invalid choice. Exiting." && exit 1;;
esac