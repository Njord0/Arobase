<div align="center">
    <img width="140px" src="../../others/logo.png"/><br/>
    Welcome to the documentation !
</div>


### Variables

Variable is one of the must important part of a programming language, depending of the language variable type might be implicit or explicit, static or dynamic.

In ***Arobase*** variable are explicitly typed and can't change during program execution.
It means that a variable can't change of type in the same scope.

To use a variable you have to declare it before, you have to use the `let` keyword for that : 
```c
fn main() : void
{
    let my_number: integer;
    my_number = 10000;
}
```
Here is you can declare a variable, note that you can declare and initialize a variable at same time : 
```c
fn main() : void
{
    let my_number: integer = 10000;
}
```