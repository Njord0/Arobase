### Arobase

<div align="center">
    <img width="140px" src="others/logo.png"/><br/>
    Arobase is a simple programming language with a c-like syntax.
</div>

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

Here a sample code which print fibonacci(10) : 
```c
fn fibonacci(a: integer) : integer
{
    if (a == 0)
    {
        return 0;
    }
               @ this is a comment
    if (a < 3) @ 1 or 2
    {
        return 1;
    }

    return fibonacci(a-2) + fibonacci(a-1);
}

@ 'main' function is needed in any program.
fn main() : void
{
    print fibonacci(10);
}
```

first step :
```
arobase -s source.aro -o out.s
```
Assembling and linking : 
```
as out.s -o out.o -mnaked-reg -msyntax=intel
ld -o out out.o -larobase 
```

## Documentation / Tutorial

You can find document or tutorial [here](docs/README.md)
