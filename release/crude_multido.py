import sys
from subprocess import call, Popen, PIPE


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


def modCart(cartfile, newcart):
  f = open('../circs/' + cartfile + '.circ', 'r')
  contents = f.read()
  f.close()

  f = open('../circs/' + cartfile + '.circ', 'w')
  contents = contents.split('\n')
  contents[2] = newcart
  for line in contents:
    f.write(line + '\n')
  f.close()


def iter(circfile, iterations, folder, threshold):
  for i in range(int(iterations)):
    try:
      p = Popen(['./main', circfile, str(i), str(threshold), folder])
      print("./main " + circfile + " " + str(i) + " " + str(threshold) + " " + folder)
      output, err = p.communicate()
    except OSError:
      pass


def outerIter():
  # for mutations in [4,5,6,7,8,9,10,12,14,16,18,20,25,30,35,40,45,50,60,70,80,90,100]:
  # for mutations in [55,65,75,85,95,100,125,150,200]:
  for mutations in [10]:
    modParams('kMutations', mutations)
    iterations = 50
    threshold = 1000
    folder = 'aa' + str(mutations)
    print(folder)
    circfile = '2bitmulstarter'
    for i in range(int(iterations)):
      try:
        print('./main ' + circfile + ' ' + str(i) + ' ' + str(threshold) + ' ' + folder)
        p = Popen(['./main', circfile, str(i), str(threshold), folder])
        output, err = p.communicate()
      except OSError:
        pass


def outerIter2():
  # for mutations in [4,5,6,7,8,9,10,12,14,16,18,20,25,30,35,40,45,50,60,70,80,90,100]:
  # for mutations in [55,65,75,85,95,100,125,150,200]:
  for breed in [4,6,8,12,16,32,64,128]:
    modParams('kBreedSample', breed)
    iterations = 10
    threshold = 1000
    folder = 'breed_count' + str(breed)
    print(folder)
    circfile = '3bitmulstarter'
    for i in range(int(iterations)):
      try:
        p = Popen(['./main', circfile, str(i), str(threshold), folder])
        output, err = p.communicate()
      except OSError:
        pass


def outerCartIter():
  carts = [
    '64$1',
    '32$2',
    '#3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1',
    '16$4',
    '#5,5,5,5,5,5,5,5,5,5,5,5,4',
    '#6,6,6,6,6,6,6,6,6,6,4',
    '#7,7,7,7,7,7,7,7,7,1',
    '#8,8,8,8,8,8,8,8',
    '#9,9,9,9,9,9,9,1',
    '#10,10,10,10,10,10,4'
  ]
  circfile = '4bitmulstarter'
  j = 1
  for cart in carts:
    modCart(circfile, cart)
    iterations = 1
    threshold = 1000
    folder = '4bitcartstretch' + str(j)
    j += 1
    for i in range(int(iterations)):
      try:
        p = call(['./main', circfile, str(i), str(threshold), folder, '&'])
        print("./main " + circfile + " " + str(i) + " " + str(threshold) + " " + folder)
        # output, err = p.communicate()
      except OSError:
        pass


if __name__ == '__main__':
  try:
    p = Popen(['./crude'])
    output, err = p.communicate()
  except OSError:
    print('adad')
    pass


  outerIter()
  # outerIter2()
  # outerCartIter()
  # iter(sys.argv[1], sys.argv[2], sys.argv[3], 1000)
