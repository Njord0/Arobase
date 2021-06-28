<div align="center">
    <img width="140px" src="../../others/logo.png"/><br/>
    Welcome to the documentation !
</div>


### Cast / Type conversion

**Arobase** is a strictly typed language, and you can't simply convert a `char` to a `byte` even if they are quite similar in their operation.

If you want to convert a `char` to a `byte` you have to use `char_to_byte` function which is located in `core.cast` module (see [here](../basics/module.md) for modules explanations).

`cast` offers 6 functions:

* `int_to_byte`
* `byte_to_int`
* `char_to_int`
* `int_to_char`
* `char_to_byte`
* `byte_to_char`

<br/>

---
> fn int_to_byte(i: integer) : byte

    This function takes an integer and return a byte.
---
> fn byte_to_int(c: byte) : integer

    This function takes a byte and return an integer.
---
> char_to_int(c: char) : integer

    This function takes a character and return an integer corresponding to the ascii value of this character.
---
> int_to_char(i: integer) : char

    This function takes an integer and return the ascii character corresponding to this integer.
---
> char_to_byte(c: char) : byte

    This function takes a character and return an integer corresponding to the ascii value of this character.
---
> byte_to_char(b: byte) : char

    This function takes a byte and return the ascii character corresponding to this byte.
