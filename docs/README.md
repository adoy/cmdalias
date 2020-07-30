# What is cmdalias

`cmdalias` is an nested alias tool that I created for myself and inspired from the [git-config alias](https://git-scm.com/book/en/v2/Git-Basics-Git-Aliases) mechanism. It lets you create context aware aliases and nested aliases (subaliases).

Why? I was tired of using too many keystrokes for commands that I'm using everyday in my shell. For example, with docker, I often want to list images or clean containers and images.

Example: 

```bash
docker
alias: d

docker images -a
alias: d i

docker network ls
alias: d n l

docker container prune --force; docker rmi $(docker images -f 'dangling=true' -q)
alias: d clear
```

With traditional aliases, you can only have one level of depth within your short names. With cmdalias, you can easily nest short names and transform `kubectl get pod` to `k g p`. More examples will be provided below.

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

- one alias file named `.cmdalias` under your **$HOME** directory
- multiple alias files in a directory named `.cmdalias/` under your **$HOME** directory. The names can be whatever you like.

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

To **dry-run** the aliases that will be created run `cmdalias -i`, they will be printed to stdout. Moreover, you can check if the syntax you wrote is correct by running `cmdalias --check-config`

When satisfied, run the following or 

```bash
source <(cmdalias -i)
```

If you execute the command `alias`, it will display the aliases that were created.

If you want **cmdalias** to create the aliases each time you login, add it to the configuration of your favorite command line interpreter.

```bash
echo "source <(cmdalias -i)" >> ~/.bashrc
```

## Configuration examples

##### Single alias

```
d = docker;
```

##### Nested/subalias

```
d = docker {
    i = images;
};
```

You can add command line arguments and options to aliases and subaliases

```
//to pipe copy, for example: echo "bar" | copy
copy = xclip -selection c;

d = docker {
    i = images;
    e = exec -it;
};
```

##### Sub shell execution
```
random-password = sh -c "dd if=/dev/urandom bs=1 count=12 2>/dev/null | base64 -w 0 | rev | cut -b 2- | rev";

// works for subaliases too
myutils = echo "MyUtils" {
    random-password = !sh -c "dd if=/dev/urandom bs=1 count=12 2>/dev/null | base64 -w 0 | rev | cut -b 2- | rev";
};
```

This would give you the possibility to execute `random-password`, `myutils` and `myutils random-password`. Note that a `!` is required for subaliases that want to execute shell commands.

##### Passing environment variables

```
//in shell:
export DIGITALOCEAN_ACCESS_TOKEN=your_token

//in .cmdalias config
doctl = docker run --rm -e "DIGITALOCEAN_ACCESS_TOKEN=$DIGITALOCEAN_ACCESS_TOKEN" doctl;
```

##### Wildcard subalias using `*`

Let's take the kubernetes command to list a single pod in a JSON output format
```
kubectl get -o json pod <some_pod>
```

We could write the following configuration:

```
k = kubectl {
    * {
        p          = pod;
        -j      = -o json;
    };
    g  = get;
};
```

And be able to execute `k g p -j`.

The `*` basically means that the subalias is permitted at any level of the command, except its root alias.

## More complex configuration examples

##### Docker

```
d = docker {
    i = images;
    l = logs -f;
    e = exec -it;
    k = kill;
    s = stop;
    r = run;
    n = network {
        l = ls;
    };
    ps = ps --format "table {{.Names}}\\t{{.Status}}\\t{{.Ports}}";
    v = volume {
        ls  = !sh -c "docker volume ls | grep -v docker | grep -v data | grep -v DRIVER | gawk '{ print $2 }'";
    };
    clean = !sh -c "docker container prune --force; docker rmi $(docker images -f 'dangling=true' -q)";
};
```

With this `docker` alias you can use commands like:

| Real command | Short command using cmdalias |
| --- | --- |
| `d i` | `docker images` |
| `d n l` | `docker network ls` |
| `d v ls` | <code>docker volume ls &#124; grep -v docker &#124; grep -v data &#124; grep -v DRIVER &#124; gawk '{ print $2 }'"</code> |
| `d r <some_image>` | `docker run <some_image>` |
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
| `kubectl apply -R -f <some_folder>` | `k a -f <some_folder>` |
| ``kubectl config set-context `kubectl config current-context` --namespace production`` | `k ns production` |
| ... | and so on... |
