<div align="center">
    <img width="140px" src="../../others/logo.png"/><br/>
    Welcome to the documentation !
</div>


### Standard types

***Arobase*** come with differents standard types which allow or not operations.

## Integer and byte

`integer` and `byte` are two types used for holding numbers, they differ in the amount of memory used to store the number : 
* `integer` is 64 bits (signed) integer
* `byte` is 8 bits (unsigned) integer

You can't add/substract/multiply/divide or do any other operations between integer and byte ! (see [here](../stdlib/cast.md) for casting)

Here is a list of supported operators for both type :

|Operator|Meaning       |
|--------|--------------|
|+       |addition      |
|-       |soustraction  |
|/       |division      |
|*       |multiplication|

## char

`char` is simple type used to store **one** character, the character must be single quoted : 'a' for example.

## string

`string` is an immuable type, it stores a string which can be printed to the screen, `string` is actually implemented as null-terminated list of characters using `.asciz` directive.

## void

In ***Arobase*** all functions must have a return type, if you don't wan't to return anything you can use the `void` type to tell that the function won't return anything.
You can't declare a variable of type `void`.