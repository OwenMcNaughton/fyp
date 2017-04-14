import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
import numpy as np
import re

f = open('../figures/Mutation_Percentage_vs_Avg_Correctness:_4bit_multiplierfull')
contents = f.read()
contents = contents.split('\n')
x = [int(i) for i in contents[0][1:-1].split(',')]
y = [float(i) for i in contents[1][1:-1].split(',')]

print(x)
print(y)
