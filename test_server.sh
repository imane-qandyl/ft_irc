#!/bin/bash

echo "=== IRC Server Network Test Suite ==="

# Test 1: Basic connection
echo "Test 1: Basic Connection"
(echo -e "Hello Server\r\n"; sleep 2) | nc localhost 6667 &
sleep 3

# Test 2: Multiple clients
echo "Test 2: Multiple Clients"
for i in {1..3}; do
    (echo -e "Client $i message\r\n"; sleep 3) | nc localhost 6667 &
done
sleep 5

# Test 3: Large message
echo "Test 3: Large Message"
(echo -e "This is a longer message to test buffer handling\r\n"; sleep 2) | nc localhost 6667 &
sleep 3

echo "Network tests completed!"
