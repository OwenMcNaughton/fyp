import click
from subprocess import call, Popen, PIPE


@click.command()
@click.argument('circfile')
@click.argument('iterations')
@click.argument('threshold')
@click.option('--messup', '-m', default=-1)
def iter(circfile, iterations, threshold, messup):
  for i in range(int(iterations)):
    try:
      p = Popen(['./main', circfile, str(i), str(threshold), str(messup)],
        stdin=PIPE, stderr=PIPE)
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
