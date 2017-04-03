import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
import numpy as np
import re


def graph(x, y, xlabel, ylabel, title):
  corrcoef = np.corrcoef(x, y)[0][1]
  plt.plot(x, y, 'ro')
  plt.plot(np.unique(x), np.poly1d(np.polyfit(x, y, 1))(np.unique(x)), label='r=' + str(corrcoef))
  plt.axis([min(x) - max(x) * .05, max(x) * 1.05, min(y) - max(y) * .05, max(y) * 1.05])
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.title(title)
  plt.legend(loc=2)
  plt.show()
  plt.savefig('../figures/' + title.replace(' ', '_') + '.png')
  f = open('../figures/' + title.replace(' ', '_'), 'w')
  f.write(str(x) + '\n')
  f.write(str(y))


f = open('.tmp')
contents = f.read().split('~')

mutes = []
completed = []
avg_evals = []
avg_evals_correct = []
avg_correct = []
avg_gates = []
for chunk in contents:
  lines = chunk.split('\n')
  if len(lines) < 3:
    continue
  for line in lines:
    if 'folder' in line:
      mutes.append(int(re.findall(r'\d+', line)[0]))
    if 'Completed' in line:
      completed.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Average Evals of Correct' in line:
      avg_evals_correct.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Average Evals of All' in line:
      avg_evals.append(float(re.findall("\d+\.\d+", line)[0]))
      print(avg_evals[-1])
    if 'Average Correctness' in line:
      avg_correct.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Gates used' in line:
      avg_gates.append(int(re.findall(r'\d+', line)[0]))
print(avg_gates)
carts = [1,2,3,4,5,6,7,8,9,10]
#
# print(avg_correct)

# graph(mutes, avg_evals, 'rand() % Mutations', 'avg evaluations',
#   'Effect of random mutation range on evaluation count for a 2bit multiplier')
# graph(carts, avg_correct, 'row size', 'final absolute correctness',
#   'Effect of cartesian plane size on final correctness')
# graph(carts, avg_gates, 'row size', 'final absolute correctness',
#   'Effect of cartesian plane size on final gate count')
graph(mutes, avg_correct, 'Breeding Sample', 'Avg correctness',
  'Effect of breeding sample size on final avg correctness for 3bit multiplier')
