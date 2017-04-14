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


def parseFolder(folder):
  files = os.listdir(folder)
  files.sort()
  for f in files:
    parseIntl(folder, [f])


def parseFolderAvg(folder):
  files = os.listdir(folder)
  files.sort()
  parseIntl(folder, files)


def parseIntl(folder, files):
  results = []
  total_completed = 0
  total = 0
  total_evals_of_completed = 0
  total_evals = 0
  total_percent = 0
  total_gates_used_in_correct = 0
  total_weighted_percent = 0

  total_count_hist = []
  weighted_total_count_hist = []
  percent_hist = []
  weighted_percent_hist = []

  total_percents = []
  total_corrects = []
  gates_useds = []

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
        gates_useds.append(int(splits[1]))
    if float(percent) == 1:
      total_completed += 1
      total_evals_of_completed += int(evaluations)
      total_gates_used_in_correct += gates_used
    if evaluations != '':
      total_evals += int(evaluations)
    total_percent += float(percent)
    total_count_hist.append([float(i) for i in contents[-5].split(',')[0:-2]])
    weighted_total_count_hist.append([float(i) for i in contents[-4].split(',')[0:-2]])
    percent_hist.append([float(i) for i in contents[-3].split(',')[0:-2]])
    weighted_percent_hist.append([float(i) for i in contents[-2].split(',')[0:-2]])
    total_percents.append([float(i) for i in contents[-2].split(',')[0:-2]][-1])
    total_corrects.append([float(i) for i in contents[-3].split(',')[0:-2]][-1])
    # print(weighted_percent_hist)

  all_weighted_percents = []
  tch = []
  for idx in range(len(total_count_hist[0])):
    tot = 0
    for arr in range(len(total_count_hist)):
      tot += total_count_hist[arr][idx]
    tch.append(int(tot / len(total_count_hist)))
  wtch = []
  for idx in range(len(weighted_total_count_hist[0])):
    tot = 0
    for arr in range(len(weighted_total_count_hist)):
      tot += weighted_total_count_hist[arr][idx]
    wtch.append(int(tot / len(weighted_total_count_hist)))
  ph = []
  for idx in range(len(percent_hist[0])):
    tot = 0
    for arr in range(len(percent_hist)):
      tot += percent_hist[arr][idx]
    ph.append((tot / len(percent_hist)))
  wph = []
  for idx in range(len(weighted_percent_hist[0])):
    tot = 0
    for arr in range(len(weighted_percent_hist)):
      tot += weighted_percent_hist[arr][idx]
    wph.append((tot / len(weighted_percent_hist)))

  print(total)
  print('folder: ' + os.path.basename(os.path.normpath(folder)))
  print('Completed: ' + str(total_completed / total))
  # print('Average Evals of Correct circuits: ' + str(total_evals_of_completed / total_completed))
  print('Average Evals of All circuits: ' + str(total_evals / total))
  print('Average Correctness: ' + str(total_percent / total))
  print('All Weighted Correctness: ' + str(total_percents))
  print('All Correctnesses: ' + str(total_corrects))
  print('Gates used: ' + str(gates_used))
  print('Skates useds: ' + str(gates_useds))
  print('total_count_hist: ' + str(tch)[1:-1])
  print('weighted_total_count_hist: ' + str(wtch)[1:-1])
  print('percent_hist: ' + str(ph)[1:-1])
  print('weighted_percent_hist: ' + str(wph)[1:-1])
  # print('Average gates of correct circuits: ' + str(total_gates_used_in_correct / total_completed))
  print('~')


if __name__ == '__main__':
  base = '../logs/cartstretch';
  # base = '../logs/aa';
  base = '../gate_types/gate_types';
  base = '../breeding/breed';
  base = '../remotelogs/4bitcartstretch';
  base = '../mutperc/mutationpercentage';

  stems = [1,2,3,4,5,6,7,8,9,10]
  # stems = ['1,2,3', '1,2,3,4,12,13']
  # stems = [1,2,4,8,16]

  # stems = [
  #   '40$1',
  #   '20$2',
  #   '#4,3,3,3,3,3,3,3,3,3,3,3,3',
  #   '10$4',
  #   '8$5',
  #   '#6,6,6,6,6,6,4',
  #   '#7,7,7,7,7,5',
  #   '#8,8,8,8,8',
  #   '#9,9,9,9,4',
  #   '#10,10,10,10'
  # ];

  # base = '../logs/breed_count';
  #
  # stems = [2,4,6,8,12,16,32,64,128];

  # base = '../logs/mutations'
  # stems = [2,3,4,5,6,7,8,9,10,12,14,16,18,20,25,30,35,40,45,50,55,60,70,80,90,100]
  # stems = [10,12,14,16,18,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,125,150,200]
  # stems = [4,5,6,7,8,9,10,12,14,16,18,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,125,150,175,200]
  # stems = [10,12,14,16,18,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,125,150,175,200]

  for i in stems:
    full = base + str(i) + '/'
    parseFolderAvg(full)
  # parse('../logs/unweight/')

  # parseFolder('../gate_types/gate_types1,2,3,4,12,13/')
