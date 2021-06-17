<div align="center">
    <img width="140px" src="../others/logo.png"/><br/>
    Welcome to this tutorial for Arobase language !
</div>

## Hello, world !

For this simple step, I suppose you successfully installed Arobase compiler (see [here](../README.md))

Copy / paste this script in a file : 
```c
fn main() : void
{
    print "Hello, world!\n";
}
```

Compile and execute the binary, "Hello, world!" will be printed.
And now a few explanations :
* `fn` is a keyword, it stands for '`f`unctio`n`'
    * it must be followed by a function declaration
    * here `main` is the name of the function, it takes no parameters
    * it's followed by the return type of the function, here `void` tells that the function returns nothing.
* The function body is between brackets, it must follow the function header.
* `print` is a keyword, it takes a list of parameters to print to the screen. (Ex: `print "Hello", "World\n";` is also a valid way to use `print`)


## Basic data types
There are 5 differents data types : 

| type | usage | example |
|------|-------|---------|
| integer| An integer | 88 |
| char | A single character | 'a'|
| byte | A smaller positive integer | 10 |
| string | An immuable string | "Hello, world!" |
| void | nothing | nothing

## Variable declaration / assignment

To declare a function you need to use the `let` keyword, for example :
```c
fn main() : void
{
    let n: integer;
    n = 100;
    let d: integer = 100;
}
```

As you can see, you can either initialize or not a variable during declaration.

You can also assign to a variable the content of another variable of the same type : 

```c
fn main() : void
{                        
                            
    let n: integer = 100;   @ This is a comment
    let d: integer = n;     @ Valid !

    let e: byte = n; @ Invalid !!!

}
```
## Operators 
There are 4 differents operators : 

|symbol|meaning|
|------|-------|
|  +   |addition|
|  -   |soustraction|
|  *   |multiplication|
|  /   |division|

As for variable assignment, you can't add/soustract/divide/multiply two variables of differents type !

```c
fn main() : void
{
    let d: integer = 20; 
    let e: integer = d+22; @ Valid!
}
```

## Print / Input

`print` and `input` are both keywords.
You can use `print` to display a string, a character or a number to the screen, for example : 

```c
fn main() : void
{
    let c: char = 'H';
    let d: string "ello, world!\n";

    print c, d, 42;
}
```

You can use `input` to ask the user to enter an integer, a byte or a char, but it doesn't work with string for now. For example :

```c
fn main() : void
{
    let c: integer;
    print "Give a number: ";
    input c;

    let d: integer;
    print "Give an other number: ";
    input d;

    print c * d, "\n";
}
```

Note that while `print` can take a list of parameters, `input` can't !

## Conditionnal statements

# Comparison operators

There are 4 comparison operators :
|symbol|meaning|
|------|-------|
| == | equal to |
| != | not equal to |
| < | less than |
| > | greater than |
| <= | less or equal to |
| >= | greater or equal to |

# if / else

```c
if (1 != 2) 
{
    print "Everything is ok\n";
}

else 
{
    print "Wow is something is going wrong\n";
}
```
Note that there is no "else if"/"elif" in Arobase, you can use nested if/else statements to simulate this behaviour.

# while loop
To repeat an action till an expression is true you can use a while loop : 
```c
fn main() : void
{
    let a: integer = 9;
    let b: integer = 0;

    while (b < 11)
    {
        print a, "*", b, " = ", a*b, "\n";
        b = b+1;
    }
}
```

## Function

As said previously to declare a function you need to use the `fn` keyword, followed by the function name, parameters and return value.

Here is an example for fibonacci function : 
```c
fn fibonacci(a: integer) : integer
{
    let n1: integer = 0;
    let n2: integer = 1;
    let tmp: integer;
    let i: integer = 0;

    while (i < a)
    {
        tmp = n2;
        n2 = n2+n1;
        n1 = tmp;
        i = i+1;
    }

    return n2;
}
```

## Import

You can use the `import` keyword to import a module. You can find module list in the documentation. Import must be done in the beginning of the program or between functions declarations but not inside function.
Here is an example : 

```c
import core.cast;

fn main() : void
{
    print int_to_char(100), "\n";
}
```

Function `int_to_char` is inside `core.cast` you can use it after importing `core.cast`.