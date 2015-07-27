#!/bin/bash

pushd ../
make
popd

make

echo
echo
echo "Running tests..."
echo

for i in `ls test_*`;
do
	./"$i";
done
