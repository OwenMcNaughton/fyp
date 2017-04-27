import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
import numpy as np
import re
import math


def fillin(xs):
  last = xs[0]
  idx = 0
  while last == 0:
    last = xs[idx]
    idx += 1
  for i in range(len(xs)):
    if xs[i] == 0:
      xs[i] = last
    else:
      last = xs[i]


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


def multiGraphOther(xss, key, xlabel, ylabel, title, fappend = '', niny = -1, naxy = -1):
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
  multiGraph(xy, key, xlabel, ylabel, title, fappend, niny, naxy)


def multiGraph(xy, key, xlabel, ylabel, title, fappend = '', niny = -1, naxy = -1):
  i = 0
  maxx = -9999999999999
  minx = 9999999999999
  maxy = -9999999999999
  miny = 9999999999999
  for (x, y) in xy:
    if len(key) > 0:
      plt.plot(x, y, label=key[i])
    else:
      plt.plot(x, y, 'k-', alpha=0.5)

    i += 1
    if max(x) > maxx:
      maxx = max(x)
    if max(y) > maxy:
      maxy = max(y)
    if min(x) < maxx:
      minx = min(x)
    if min(y) < miny:
      miny = min(y)
  if niny == -1:
    plt.axis([minx - maxx * .05, maxx * 1.05, miny - maxy * .05, max(y) * 1.05])
  else:
    plt.axis([minx - maxx * .05, maxx * 1.05, niny, naxy])

  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.title(title)
  if len(key):
    plt.legend(loc=4)
  plt.show()
  plt.savefig('../figures/' + (title + fappend).replace(' ', '_') + '.png')
  f = open('../figures/' + (title + fappend).replace(' ', '_'), 'w')
  f.write(str(x) + '\n')
  f.write(str(y))


def graph(x, y, xlabel, ylabel, title, fappend = '', errorbars = [], niny = -1, naxy = -1):
  corrcoef = np.corrcoef(x, y)[0][1]
  if len(errorbars) > 0:
    plt.errorbar(x, y, fmt='ro', yerr=errorbars, ecolor='g')
  else:
    plt.plot(x, y, 'ro')
  plt.plot(np.unique(x), np.poly1d(np.polyfit(x, y, 1))(np.unique(x)), label='r=' + str(corrcoef))

  if niny == -1:
    plt.axis([min(x) - max(x) * .05, max(x) * 1.05, min(y) - max(y) * .05, max(y) * 1.05])
  else:
    plt.axis([min(x) - max(x) * .05, max(x) * 1.05, niny, naxy])
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.title(title)
  plt.legend(loc=1)
  plt.show()
  plt.savefig('../figures/' + (title + fappend).replace(' ', '_') + '.png')
  f = open('../figures/' + (title + fappend).replace(' ', '_'), 'w')
  f.write(str(x) + '\n')
  f.write(str(y))


def graphSpec(xl, y, xlabel, ylabel, title, fappend = '', errorbars = [], niny = -1, naxy = -1):
  x = range(len(xl))
  corrcoef = np.corrcoef(x, y)[0][1]
  if len(errorbars) > 0:
    plt.errorbar(x, y, fmt='ro', yerr=errorbars, ecolor='g')
  else:
    plt.plot(x, y, 'ro')
  plt.plot(np.unique(x), np.poly1d(np.polyfit(x, y, 1))(np.unique(x)), label='r=' + str(corrcoef))

  plt.xticks(x, xl)

  if niny == -1:
    plt.axis([min(x) - max(x) * .05, max(x) * 1.05, min(y) - max(y) * .05, max(y) * 1.05])
  else:
    plt.axis([min(x) - max(x) * .05, max(x) * 1.05, niny, naxy])
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.title(title)
  plt.legend(loc=1)
  plt.show()
  plt.savefig('../figures/' + (title + fappend).replace(' ', '_') + '.png')
  f = open('../figures/' + (title + fappend).replace(' ', '_'), 'w')
  f.write(str(x) + '\n')
  f.write(str(y))


f = open('.tmp')
contents = f.read().split('~')

mutes = []
completed = []
avg_evals = []
avg_evals_correct = []
avg_correct = []
avg_weighted_correct = []
final_correctness = []
avg_gates = []
total_hist = []
weighted_total_hist = []
percent_hist = []
weighted_percent_hist = []
errors = []
total_corrects = []
skates_used = []
for chunk in contents:
  lines = chunk.split('\n')
  if len(lines) < 3:
    continue
  for line in lines:
    # if 'folder' in line:
    #   mutes.append(int(re.findall(r'\d+', line)[0]))
    if 'Completed' in line:
      completed.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Average Evals of Correct' in line:
      avg_evals_correct.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Average Evals of All' in line:
      avg_evals.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Average Correctness' in line:
      avg_correct.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'Average Weighted Correctness' in line:
      avg_weighted_correct.append(float(re.findall("\d+\.\d+", line)[0]))
    if 'All Correctnesses' in line:
      total_corrects.append([float(i) for i in (re.findall("\d+\.\d+", line))])
    if 'Gates used' in line:
      avg_gates.append(int(re.findall(r'\d+', line)[0]))
    if 'Skates useds' in line:
      # print([int(i) for i in(re.findall(r'\d+', line))])
      skates_used.append([int(i) for i in(re.findall(r'\d+', line))])
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
      line2 = line2[0:198]
      fillin(line2)
      # print(str(len(line2)) + ' ' + str(min(line2)) + ' ' + str(line2[0:20]))
      weighted_percent_hist.append(line2)
      final_correctness.append(line2[-1])


print(avg_gates)
print(str(sum(avg_gates) / len(avg_gates)) + ' ' + str(np.var(avg_gates)))
print(final_correctness)
print(str(sum(final_correctness) / len(final_correctness)) + ' ' + str(np.var(final_correctness)))

carts = [1,2,3,4,5,6,7,8,9,10]
muts = ['1%','2%','3%','4%','5%','6%','7%','8%','9%','10%']
breeds = [1,2,4,8,16]

errorbars = []
# avg_gates = []
for datums in total_corrects:
  print(datums)
  avg = sum(datums) / len(datums)
  # avg_gates.append(avg)
  stddev = 0
  for d in datums:
    stddev += (d - avg) * (d - avg)
  stddev = math.sqrt(stddev / len(datums))
  errorbars.append(stddev / 2)
gates_errorbars = []
# avg_gates = []
for datums in skates_used:
  print(datums)
  avg = sum(datums) / len(datums)
  # avg_gates.append(avg)
  stddev = 0
  for d in datums:
    stddev += (d - avg) * (d - avg)
  stddev = math.sqrt(stddev / len(datums))
  gates_errorbars.append(stddev / 2)

print(final_correctness)
print(avg_gates)

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

# selection = [1,1,0,1,0,0,0,1,0,1]
# # selection = [1,1,1,1,1,1,1,1,1,1]
# newcarts = []
# new_hist = []
# for i in range(len(selection)):
#   if selection[i] == 1:
#     newcarts.append(carts[i])
#     new_hist.append(weighted_percent_hist[i])

# smoothJagBig(weighted_percent_hist)
# multiGraphOther(weighted_percent_hist, carts, 'Generation', 'Best correctness',
#     'Effect of cartesian plane row count on correctness progress for 4bit multiplier')
smoothJagBig(weighted_percent_hist)
# multiGraphOther(new_hist, newcarts, 'Generation', 'Best correctness',
#   'Cartesian plane size vs correctness progress: 4bit multiplier', '1half')
# multiGraphOther(weighted_percent_hist, [1], 'Generation', 'Best correctness',
#   'Correctness progress: 2bit multiplier', 'full')
# multiGraphOther(weighted_percent_hist, muts, 'Generation', 'Best Correctness',
#     'Correctness progress across mutation percentage: 4bit multiplier', 'full')
# graph(carts, avg_correct, 'Mutation%', 'Avg Correctness',
#     'Mutation Percentage vs Avg Correctness: 4bit multiplier', 'full')
# graph(carts, avg_correct, 'Mutation%', 'Avg Correctness',
#     'Mutation Percentage vs Avg Correctness: 4bit multiplier', 'full_error2', errorbars)
# graph(carts, final_correctness, 'Row Count', 'Avg Correctness',
#     'Row Count vs Avg Correctness: 4bit multiplier', 'full_error', errorbars)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#     'Allowable Gate Types = {AND,OR,XOR}', 'full2', 0.65, 1.00)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#     'Allowable Gate Types = {AND,OR,XOR,NAND,NOR,XNOR}', 'full2', 0.65, 1.00)
# graph(breeds, final_correctness, 'Crossbreed Count', 'Final correctness',
#     'What even is ', 'ffff', errorbars)
# graph(carts, avg_gates, 'Row Count', 'Avg Final Gates',
#     'Row Count vs Avg Final Gates: 4bit multiplier', 'ff', errorbars, 10, 40)
# graph(carts, avg_gates, 'Mutation%', 'Avg Correctness',
#   'tmp', 'full_error2', errorbars)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#   'Correctnessfsdsdf no brid', '1half', niny = 0.65, naxy = 0.95)
# graph(carts, final_correctness, 'Row Count', 'Avg Correctness',
#   'Row Count vs Avg Correctness: 4bit adder', 'full_error', errorbars)
# graph(carts, avg_gates, 'Row Count', 'Avg Gates',
#   'Row Count vs Avg Final Gates: 4bit adder', 'full', gates_errorbars, niny=10, naxy=20)
# graph(carts, final_correctness, 'Row Count', 'Avg Correctness',
#   'Row Count vs Avg Correctness: 4bit multiplier', 'full_error_dos', errorbars)
# graph(carts, avg_gates, 'Row Count', 'Avg Gates',
#   'Row Count vs Avg Final Gates: 4bit multiplier', 'full', gates_errorbars)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#   'No breeding: 4bit multiplier', 'full2', niny=0.7, naxy=1.0)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#   'Normal breeding: 4bit multiplier', 'full2', niny=0.7, naxy=1.0)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#   'Multi breeding: 4bit multiplier', 'full2', niny=0.7, naxy=1.0)
# graphSpec(['No cross-breeding','Cross-breeding','Multi-breeding'], avg_gates,
#   'Offspring Generation Methods', 'Avg Gates',
#   'Comparison of offspring generation methods (gates)', 'full', gates_errorbars,
#   niny=27, naxy=36)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#   'No breeding: 4bit adder', 'full', niny=0.6, naxy=1.0)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#   'Normal breeding: 4bit adder', 'full', niny=0.6, naxy=1.0)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#   'Multi breeding: 4bit adder', 'full', niny=0.6, naxy=1.0)
# graphSpec(['No cross-breeding','Cross-breeding','Multi-breeding'], final_correctness,
#   'Offspring Generation Methods', 'Avg Correctness',
#   'Comparison of offspring generation methods, 4bit Multiplier', 'full', errorbars,
#   niny=0.9, naxy=1.0)
# graphSpec(['No cross-breeding','Cross-breeding','Multi-breeding'], avg_gates,
#   'Offspring Generation Methods', 'Avg Gates',
#   'Comparison of offspring generation methods, 4bit Multiplier (gates)', 'full', gates_errorbars,
#   niny=26, naxy=38)
# graphSpec(['No cross-breeding','Cross-breeding','Multi-breeding'], final_correctness,
#   'Offspring Generation Methods', 'Avg Correctness',
#   'Comparison of offspring generation methods, 4bit adder', 'full', errorbars,
#   niny=0.9, naxy=1.0)
# graphSpec(['No cross-breeding','Cross-breeding','Multi-breeding'], avg_gates,
#   'Offspring Generation Methods', 'Avg Gates',
#   'Comparison of offspring generation methods, 4bit adder (gates)', 'full', gates_errorbars,
#   niny=8, naxy=14)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#     'Allowable Gate Types = {AND,OR,XOR}, 4bit Multiplier', 'full3', 0.65, 1.00)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#     'Allowable Gate Types = {AND,OR,XOR,NAND,NOR,XNOR}', 'full3', 0.65, 1.00)
# multiGraphOther(weighted_percent_hist, [], 'Generation', 'Best correctness',
#     'tits', 'full3', 0.65, 1.00)
# graph(carts, final_correctness, 'Row Count', 'Avg Correctness',
#   'Row Count vs Avg Correctness: 3bit multiplier', 'full_error_dos', errorbars)
# graph(carts, avg_gates, 'Row Count', 'Avg Gates',
#   'Row Count vs Avg Final Gates: 3bit multiplier', 'full', gates_errorbars)
# graph(carts, final_correctness, 'Row Count', 'Avg Correctness',
#   'Row Count vs Avg Correctness: 4bit multiplier with frozen first row', 'full_error_dos', errorbars)
# graph(carts, avg_gates, 'Row Count', 'Avg Gates',
#   'Row Count vs Avg Final Gates: 4bit multiplier with frozen first row', 'full', gates_errorbars)
