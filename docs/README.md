# What is cmdalias

`cmdalias` is an alias tool inspired from git-config that I created for myself. Moreover, it lets you create
nested aliases.

Why? I was tired of using too many keystrokes for commands that I'm using everyday in my shell. For example, with docker, I often want to list images or clean containers and images.

Example: 

```bash
docker
alias: d

docker images -a
alias: d i

docker container prune --force; docker rmi $(docker images -f 'dangling=true' -q)
alias: d c
```

## Installation

Compile the binary and put it in your `$PATH`

```bash
./configure [--prefix=/path/to/location]
make && make install
```

**cmdalias** will install itself by default with the prefix `/usr/local` with the binary located at `/usr/local/bin`.

## Usage

##### Configure your commands

Choose your configuration method, either:

- one alias file named `.cmdalias` under your $HOME directory
- multiple alias files in a directory named `.cmdalias/` under your *$HOME* directory. The names can be whatever you like. The names can be whatever you like. The names can be whatever you like. The names can be whatever you like.

For example:

```
$HOME/
│   .cmdalias
└───.cmdalias/
│   │   docker
│   │   k18n
│   │   utilities
```

##### Run

To **dry-run** the aliases that will be created run `cmdalias -i`

When satisfied, run the following or add it to the configuration of your favorite command line interpreter.

```bash
source <(cmdalias -i)
```

If you execute the command `alias`, it will display the aliases that were created.

## More complex configuration examples

##### Docker

`.cmdalias/docker`
```
d = docker {
    i = images;
    l = logs -f;
    e = exec -it;
    k = kill;
    s = stop;
    r = run;
    n = network;
    r = restart;
    ps = ps --format "table {{.Names}}\\t{{.Status}}\\t{{.Ports}}";
    v = volume {
        ls  = !sh -c "docker volume ls | grep -v docker | grep -v data | grep -v DRIVER | gawk '{ print $2 }'";
    };
    clean = !sh -c "docker container prune --force; docker rmi $(docker images -f 'dangling=true' -q)";
    rmi {
        list  = !sh -c "gawk '{ print $1\":\"$2 }' | sort | uniq | xargs -n1 docker rmi";
        idlist = !sh -c "gawk '{ print $3 }' | sort | uniq | xargs -n1 docker rmi";
    };
};
```

With this `docker` alias you can use commands like:

| Real command | Short command using cmdalias |
| --- | --- |
| `d i` | `docker images` |
| `d v ls` | <code>docker volume ls &#124; grep -v docker &#124; grep -v data &#124; grep -v DRIVER &#124; gawk '{ print $2 }'"</code> |
| ... | and so on... |

##### Kubernetes

```
k = kubectl {
    * {
        p          = pod;
        i          = ingress;
        d          = deploy;
        s          = svc;
        pp         = podpreset;
        -a         = --all-namespaces;
        -json      = -o json;
        -yaml,-yml = -o yaml;
    };
    ns = config set-context `kubectl config current-context` --namespace;
    gc = config get-contexts;
    uc = config use-context;
    cc = config current-context;
    g  = get;
    d, desc  = describe;
    a, apply = apply {
        -f = -R -f;
    };
    e = edit;
    tail = !kail;
};
```

With this `kubectl` alias you can use commands like:

| Real command | Short command using cmdalias |
| --- | --- |
| `kubectl config get-contexts` | `k gc` |
| `kubectl config use-context production` | `k uc production` |
| `kubectl get pod --all-namespaces` | `k g p -a` |
| `kubectl apply -R -f somefolder` | `k a -f somefolder` |
| ``kubectl config set-context `kubectl config current-context` --namespace production`` | `k ns production` |
| ... | and so on... |

##### Normal aliases or subshell executions are also possible

```
random-password = sh -c "dd if=/dev/urandom bs=1 count=12 2>/dev/null | base64 -w 0 | rev | cut -b 2- | rev";
```

##### Passing environment variables

```
export DIGITALOCEAN_ACCESS_TOKEN=your_token
doctl = docker run --rm -e "DIGITALOCEAN_ACCESS_TOKEN=$DIGITALOCEAN_ACCESS_TOKEN" doctl;
```
