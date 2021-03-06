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

## Arrays declaration

To declare an array you need to specify the type followed by `[x]` where x is the numbers of elements : 
```c
fn main() : void
{
    let a: integer[3] = [1, 2, 3];

    print a[0], a[1], a[2], "\n";
}
```

You can't declare array of strings or arrays of structures.

## Structures declaration

To declare a structure you have to use the `struct` keyword followed by the structure name. 
The differents members of the structure are then declared in braces.

```c
Player {
    name: string,
    stamina: integer,
    health: integer
}
```
Here we declare a structure `Player` with 3 members.

Structures variables are initialised the same as any other variable using the let keyword and the structure name.

```c
fn main() : void
{
    let player: Player;
    player.name = "username";
    player.stamina = 0;
    player.health = 100;
}
```
Quite easy, isn't it ?