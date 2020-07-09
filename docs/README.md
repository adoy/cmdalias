# What is cmdalias

`cmdalias` is a tool that I created for myself because I was tired of using too many keystrokes for commands that i'm using everyday in my shell. For example I often want to list images from my docker.

Usually to do that I would have to type the following command

```
$ docker images
```

Using a simple bash alias `alias d=docker` I will have to type this command

```
$ d images
```

But using cmdalias I can have commands like 

```
$ d i
```

## Installation

Compile the binary and put it in your `$PATH`

```
./configure
make
make install
```

By default make install copy the binary to `/usr/local/bin`.

Add the following line to your `~/.bashrc` or equivalent file.

```
source <(cmdalias -i)
```

## Configure your commands

Create a file located in your `$HOME` directory named `.cmdalias`

```
d = docker {
  i = image;
};
```

To reload the configuration just reload your `~/.bashrc` file your current shell.

## More complexe configuration example

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

With this `kubectl` alias I can use commands like

| Real command | Short command using cmdalias |
| --- | --- |
| `kubectl config get-contexts` | `k gc` |
| `kubectl config use-context production` | `k uc production` |
| `kubectl get pod --all-namespaces` | `k g p -a` |
| `kubectl apply -R -f somefolder` | `k a -f somefolder` |
| ``kubectl config set-context `kubectl config current-context` --namespace production`` | `k ns production` |


