### Arobase

<div align="center">
    <img width="140px" src="others/logo.png"/><br/>
    Arobase is a simple programming language with a c-like syntax.
</div>

## About
Arobase is a simple programming language, actual implementation is a compiler written in C

The language is meant to be easy to use and easy to learn! 

## Requirements
* gcc
* gas (gnu as)
* ld
* A 64 bits linux distribution (generated assembly is for x64 architecture)

## Installation
```
$ git clone https://github.com/njord0/Arobase
$ cd Arobase/arobase
$ make
$ make install
```

## Example
Here a sample code which prints fibonacci(10), copy/paste it to a file named `source.aro`
```c
fn fibonacci(a: integer) : integer
{
    if (a == 0)
    {
        return 0;
    }
               @ this is a comment
    if (a <= 2) @ 1 or 2
    {
        return 1;
    }

    return fibonacci(a-2) + fibonacci(a-1);
}
@@  This is a comment on multiple lines
    'main' function is needed in any program.
@@
fn main() : void
{
    print fibonacci(10);
}
```

first step :
```
$ arobase -s source.aro -o out.s
```
Assembling and linking : 
```
$ as out.s -o out.o -mnaked-reg -msyntax=intel
$ ld -o out out.o -larobase 
```

For more details see the `--help` option: 
```
$ arobase --help
Usage: ./arobase -s source_file [options]
        options:
        -o              Output file name, default is 'out.s'
        --no-start      Tell the compiler to not add a '_start' function
```

## Documentation / Tutorial

You can find documentation and tutorial ~~[here](docs/README.md)~~ [here](https://arobase.njord.re)
