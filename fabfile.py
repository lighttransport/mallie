from fabric.api import env, cd, run, local

env.hosts = ['vsp']
env.use_ssh_config = True       # Read local .ssh/config

def get_mallie_dir():
    hostname = env.host
    if hostname == 'vsp':
        return '~/work/mallie/'
    else:
        return '~/work/mallie/'

def prepare():
    with cd(get_mallie_dir()):
        run('git pull')

def configure():
    with cd(get_mallie_dir()):
        if env.host == 'vsp': # CentOS
            run('./scripts/setup_linux.sh')

def build():
    with cd(get_mallie_dir()):
        run('make clean')
        run('make')

def deploy():
    pass

def host_type():
    run('uname -s')
