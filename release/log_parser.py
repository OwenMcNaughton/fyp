import click
from subprocess import call, Popen, PIPE
import os


def modParams(param, new_value):
  f = open('../src/params', 'r')
  contents = f.read()
  f.close()

  f = open('../src/params', 'w')
  contents = contents.split('\n')
  for line in contents:
    splits = line.split(':')
    print(splits)
    if len(splits) > 1:
      if splits[0] == param:
        splits[1] = ' ' + str(new_value)
      f.write(splits[0] + ':' + splits[1] + '\n')
    else:
      f.write(line)
  f.close()


def parse(folder):
  files = os.listdir(folder)
  files.sort()
  results = []
  total_completed = 0
  total = 0
  total_evals_of_completed = 0
  total_evals = 0
  total_percent = 0
  total_gates_used_in_correct = 0
  for fname in files:
    total += 1
    f = open(folder + fname)
    contents = f.read().split('\n')
    percent = ''
    evaluations = ''
    gates_used = 0
    for line in contents:
      splits = line.split(':')
      if splits[0] == 'percent':
        percent = splits[1]
      if splits[0] == 'evaluations':
        evaluations = splits[1]
      if splits[0] == 'gates_used':
        gates_used = int(splits[1])
    if float(percent) == 1:
      total_completed += 1
      total_evals_of_completed += int(evaluations)
      total_gates_used_in_correct += gates_used
    if evaluations != '':
      total_evals += int(evaluations)
    total_percent += float(percent)

  print(total)
  print('folder: ' + os.path.basename(os.path.normpath(folder)))
  print('Completed: ' + str(total_completed / total))
  # print('Average Evals of Correct circuits: ' + str(total_evals_of_completed / total_completed))
  print('Average Evals of All circuits: ' + str(total_evals / total))
  print('Average Correctness: ' + str(total_percent / total))
  print('Gates used: ' + str(gates_used))
  # print('Average gates of correct circuits: ' + str(total_gates_used_in_correct / total_completed))
  print('~')


if __name__ == '__main__':
  base = '../logs/cartstretch';

  stems = [
    '40$1',
    '20$2',
    '#4,3,3,3,3,3,3,3,3,3,3,3,3',
    '10$4',
    '8$5',
    '#6,6,6,6,6,6,4',
    '#7,7,7,7,7,5',
    '#8,8,8,8,8',
    '#9,9,9,9,4',
    '#10,10,10,10'
  ];

  base = '../logs/breed_count';

  stems = [2,4,6,8,12,16,32,64,128];

  # base = '../logs/mutations'
  # stems = [2,3,4,5,6,7,8,9,10,12,14,16,18,20,25,30,35,40,45,50,55,60,70,80,90,100]
  # stems = [10,12,14,16,18,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,125,150,200]
  # stems = [4,5,6,7,8,9,10,12,14,16,18,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,125,150,175,200]
  # stems = [10,12,14,16,18,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,125,150,175,200]

  for i in stems:
    full = base + str(i) + '/'
    parse(full)
  # parse('../logs/unweight/')
