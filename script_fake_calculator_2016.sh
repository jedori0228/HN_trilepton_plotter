#!/bin/bash
for i in 0.5 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.5 5.0
do
  root -l -b -q "fake_calculator_2016.C($i)"
done