#!/usr/bin/env python


import click
from pexpect import pxssh
import getpass

@click.command()
@click.option('-n', '--dry-run', is_flag=True)
@click.argument("crate", type=click.IntRange(3,5))
def cli(dry_run, crate):
    """"""
    password = getpass.getpass('Password: ')

    for i in range(1,6):
        hostname = f'np04-wib-{crate}0{i}'
        s = pxssh.pxssh()
        s.login(hostname, 'root', password)
        s.sendline('uptime')   # run a command
        s.prompt()             # match the prompt
        print(hostname, s.before.decode('utf-8'))        # print everything before the prompt.
        if not dry_run:
            s.sendline('reboot')   # run a command
            s.prompt()
            print(hostname, s.before.decode('utf-8'))        # print everything before the prompt.
        s.logout()

if __name__ == "__main__":
    cli()
