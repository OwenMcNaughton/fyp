import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
import numpy as np
import re


def smoothJagBig(xss):
  for xs in xss:
    smoothJag(xs)


def smoothJag(xs):
  m = xs[0]
  for i in range(len(xs)):
    if xs[i] > m:
      m = xs[i]
    else:
      xs[i] = m


def multiGraphArse(xss, key, xlabel, ylabel, title):
  xy = []
  for xs in xss:
    i = 0
    innerx = []
    innery = []
    for x in xs:
      innerx.append(i)
      innery.append(x)
      i += 1
    xy.append((innerx, innery))
  multiGraph(xy, key, xlabel, ylabel, title)


def multiGraph(xy, key, xlabel, ylabel, title):
  i = 0
  maxx = -9999999999999
  minx = 9999999999999
  maxy = -9999999999999
  miny = 9999999999999
  for (x, y) in xy:
    plt.plot(x, y, label=key[i])
    i += 1
    if max(x) > maxx:
      maxx = max(x)
    if max(y) > maxy:
      maxy = max(y)
    if min(x) < maxx:
      minx = min(x)
    if min(y) < miny:
      miny = min(y)
  plt.axis([minx - maxx * .05, maxx * 1.05, miny - maxy * .05, max(y) * 1.05])
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.title(title)
  plt.legend(loc=4)
  plt.show()
  plt.savefig('../figures/' + title.replace(' ', '_') + '.png')
  f = open('../figures/' + title.replace(' ', '_'), 'w')
  f.write(str(x) + '\n')
  f.write(str(y))


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
final_correctness = []
avg_gates = []
total_hist = []
weighted_total_hist = []
percent_hist = []
weighted_percent_hist = []
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
    if 'Average Correctness' in line:
      avg_correct.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Gates used' in line:
      avg_gates.append(int(re.findall(r'\d+', line)[0]))
    if 'total_count_hist' in line:
      line2 = line.split(':')[1]
      line2 = line2.split(',')[0:-2]
      line2 = [float(i) for i in line2]
      total_hist.append(line2)
    if 'weighted_total_count_hist' in line:
      line2 = line.split(':')[1]
      line2 = line2.split(',')[0:-2]
      line2 = [float(i) for i in line2]
      weighted_total_hist.append(line2)
    if 'percent_hist' in line:
      line2 = line.split(':')[1]
      line2 = line2.split(',')[0:-2]
      line2 = [float(i) for i in line2]
      percent_hist.append(line2)
    if 'weighted_percent_hist' in line:
      line2 = line.split(':')[1]
      line2 = line2.split(',')[0:-2]
      line2 = [float(i) for i in line2]
      weighted_percent_hist.append(line2)
      final_correctness.append(line2[-1])

carts = [1,2,3,4,5,6,7,8,9,10]

# print(avg_correct)

# graph(mutes, avg_evals, 'rand() % Mutations', 'avg evaluations',
#   'Effect of random mutation range on evaluation count for a 2bit multiplier')
# graph(carts, avg_correct, 'row size', 'final absolute correctness',
#   'Effect of cartesian plane size on final correctness')
# graph(carts, avg_gates, 'row size', 'final absolute correctness',
#   'Effect of cartesian plane size on final gate count')
# graph(mutes, avg_correct, 'Breeding Sample', 'Avg correctness',
#   'Effect of breeding sample size on final avg correctness for 3bit multiplier')

# graph(carts, final_correctness, 'Cartesian plane rows', 'Final correctness',
#     'Effect of cartesian plane size on final correctness for 4bit multiplier')
# graph(carts, avg_gates, 'Cartesian plane rows', 'Final gate count',
#     'Effect of cartesian plane size on final gate count for 4bit multiplier')

selection = [1,1,0,1,0,0,0,1,0,1]
newcarts = []
new_hist = []
for i in range(len(selection)):
  if selection[i] == 1:
    newcarts.append(carts[i])
    new_hist.append(weighted_percent_hist[i])

# smoothJagBig(weighted_percent_hist)
# multiGraphArse(weighted_percent_hist, carts, 'Generation', 'Best correctness',
#     'Effect of cartesian plane row count on correctness progress for 4bit multiplier')
smoothJagBig(weighted_percent_hist)
multiGraphArse(new_hist, newcarts, 'Generation', 'Best correctness',
  'Effect of cartesian plane row count on correctness progress for 4bit multiplier')
