<div align="center">
    <img width="140px" src="../others/logo.png"/><br/>
    Welcome to the documentation !
</div>

### Functions

## Function declaration

To declare a function you have to use the `fn` keyword followed by the function name, parameters and return type, here is an example : 
```c
fn main() : void
{

}
```
Here we declare a function named main that return `void` (nothing). 

Function can return : `integer`, `char`, `byte`, `void` but not `string`.

Function allows calling itself (recursivity).

Here is an example of function with parameters : 
```c
fn pow(a: integer, b: integer) : integer
{

    let i: integer = 0;
    let ret: integer = 1;

    while (i < b)
    {
        ret = ret*a;
        i += 1;
    }

    return ret;
}
```


## Function call

To call a function you can do : 

```c
fn inc(a: integer) : integer
{
    return a+1;
}

fn main() : void
{
    let a: integer = inc(41);
}
```

Function can be used in complex expression, if a function returns an `integer` then you can apply all operations that an integer support.

```c
fn inc(a: integer) : integer
{
    return a+1;
}

fn main() : void
{
    if (inc(41) != 42)
    {
        print "Something went wrong\n";
    }
}
```