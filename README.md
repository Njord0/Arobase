### Arobase

<div align="center">
    <img width="140px" src="others/logo.png"/><br/>
    Arobase is a simple programming language with a c-like syntax.<br/><br/>
</div>

<div align="center">
    <a href="https://github.com/Njord0/Arobase/tree/main/arobase">
        <img src="https://img.shields.io/tokei/lines/github/njord0/Arobase?color=green&label=lines%20of%20code"/></a>
    <a href="https://github.com/Njord0/Arobase/tree/main/arobase">
        <img src="https://img.shields.io/github/last-commit/njord0/Arobase"/></a>
    <img src="https://img.shields.io/github/stars/njord0/Arobase.svg"/>
    <img src="https://img.shields.io/lgtm/grade/cpp/g/Njord0/Arobase.svg?logo=lgtm&logoWidth=18"/>
    <img src="https://img.shields.io/lgtm/alerts/g/Njord0/Arobase.svg?logo=lgtm&logoWidth=18"/>

</div>

## About
Arobase is a simple programming language, current implementation is a compiler written in C

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

Compilation :
```
$ arobase -s source.aro -o out
```

For more details see the `--help` option: 
```
$ arobase --help
Usage: ./arobase -s source_file [options]
Options:
  -o            Output file name, default is 'out'
  --no-start    Tell the compiler to not add a '_start' function
  --assembly    Output assembly instead of executable
```

## Documentation / Tutorial

You can find documentation and tutorial [here](https://arobase.njord.re)
