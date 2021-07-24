<div align="center">
    <img width="140px" src="../../others/logo.png"/><br/>
    Welcome to the documentation !
</div>


### Cast / Type conversion

**Arobase** is a strictly typed language, and you can't simply convert a `char` to a `byte` even if they are quite similar in their operation.

If you want to convert a `char` to a `byte` you have to use `char_to_byte` function which is located in `core.cast` module (see [here](../basics/module.md) for modules explanations).

`cast` offers 3 functions:

* `to_byte`
* `to_int`
* `to_char`

<br/>

---
> fn to_byte(i: integer) : byte

    This function takes an integer and return a byte.
---
> fn to_int(c: byte) : integer

    This function takes a byte and return an integer.
---
> to_int(c: char) : integer

    This function takes a character and return an integer corresponding to the ascii value of this character.
---
> to_char(i: integer) : char

    This function takes an integer and return the ascii character corresponding to this integer.
---
> to_byte(c: char) : byte

    This function takes a character and return an integer corresponding to the ascii value of this character.
---
> to_char(b: byte) : char

    This function takes a byte and return the ascii character corresponding to this byte.
