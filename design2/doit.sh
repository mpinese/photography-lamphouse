#!/bin/bash

if [ ! -e design2_optim1.shl ]; then
	python design2_optim1.py design2_optim1.shl 50 0.2 5 1 5 2 5 3 5 4 10 5 10 8
else
	python design2_optim1.py design2_optim1.shl
fi
