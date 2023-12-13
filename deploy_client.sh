#!/bin/bash

# Number of client instances to run in parallel
NUM_CLIENTS=10


# File to transfer
FILE_TO_TRANSFER="$1"


DELAY_SECONDS=1


# Server IP and port
SERVER_IP_PORT="127.0.0.1:8080"

# Run the specified number of client instances in parallel
for ((i = 1; i <= NUM_CLIENTS; i++)); do
  sleep $DELAY_SECONDS && ./build/client -c $SERVER_IP_PORT -f $FILE_TO_TRANSFER  -o build/output/ &
done

# Wait for all client instances to finish
wait

echo "All client instances have finished."

