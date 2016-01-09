#!/bin/bash

if [ ! -e design1_optim1.shl ]; then
	python design1_optim1.py design1_optim1.shl 50 0.2 5 1 5 2 5 3 5 4 10 5 10 8
else
	python design1_optim1.py design1_optim1.shl
fi

if [ ! -e design1_optim2.shl ]; then
	python design1_optim2.py design1_optim2.shl 50 0.2 5 1 5 2 5 3 5 4 10 5 10 8
else
	python design1_optim2.py design1_optim2.shl
fi
