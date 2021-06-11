<div align="center">
    <img width="140px" src="../../others/logo.png"/><br/>
    Welcome to the documentation !
</div>

### Input/Output

In a simple program there is two ways to interacts with user :

* To print text on the screen
* To ask user to input thing on the keyboard

***Arobase*** uses for that two keywords : `input` and `print`

## Input

To ask user to input something you can use the `input` keyword, it works with `integer`, `char` and `byte`. `input` must be followed by the name of variable in which we want to store the value (`input` doesn't work with `string`).
```c
fn main() : void
{
    let c: integer;
    input c;
}
```

But as you can see the user doesn't know what the program is waiting for, the `print` keyword can be used for that purpose.

## Print

`print` is also a keyword, but it works differenlty of `input` since `print` can accept a list of arguments to print : 

```c
fn main() : void
{
    let c: integer;
    print "Choose a number between 1 and 100000 : ", "\n";
    input c;
}
```
There is no limits of elements that can be printed, and they can be of differents types : 

```c
fn main() : void
{
    let a: integer = 10;
    let b: integer = 20;

    print a, "*", b, " = ", a*b, "\n";
}
```

This code will output : 
> 10*20 = 200
