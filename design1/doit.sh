#!/bin/bash

if [ ! -e design1.shl ]; then
	python optim1.py design1.shl 50 0.2 5 1 5 2 5 3 5 4 10 5 10 8
else
	python optim1.py design1.shl
fi
