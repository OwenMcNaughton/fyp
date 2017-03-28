import click
from subprocess import call, Popen, PIPE


@click.command()
@click.argument('circfile')
@click.argument('iterations')
@click.argument('folder')
@click.option('--threshold', '-t', default=1000)
def iter(circfile, iterations, folder, threshold):
  for i in range(int(iterations)):
    try:
      p = Popen(['./main', circfile, str(i), str(threshold), folder])
      print("./main " + circfile + " " + str(i) + " " + str(threshold) + " " + folder)
      output, err = p.communicate()
    except OSError:
      pass


if __name__ == '__main__':
  try:
    p = Popen(['cmake', '..'])
    output, err = p.communicate()
    p = Popen(['make'])
    output, err = p.communicate()
  except OSError:
    pass

  iter()
