<div align="center">
    <img width="140px" src="../../others/logo.png"/><br/>
    Welcome to the documentation !
</div>

### Conditionnal statements


## if ... else

If statements can be used to execute an action if a boolean expression is true, if/else syntax is similar to C syntax :

```c
fn main() : void
{
    let c: integer = 10;

    if (c == 10)
    {
        print "Everything is ok\n";
    }
    else
    {
        print "What's going on here ?\n";
    }

}
```

## while

While loop can be used to repeat an action till a boolean expression is true, for example : 

```c
fn main() : void
{
    let i: integer = 0;

    while (i < 10)
    {
        print i, "\n";
        i = i+1;
    }
}
```
This code will print all numbers from 0 to 9, we can translate it to : 
print `i` and increment `i` by one until `i` is strictly inferior to 10

## For

The syntax of a for loop is : 
```
for (initialization; condition; update;) {}
```

For example : 
```c
fn main() : void
{
    for (let i: integer = 0; i < 10; i = i+1;)
    {
        print i, "\n";
    }
}
```

In initialization you can declare a variable of type `integer` only.

You can also initialize a variable declared previously : 
```c
fn main() : void
{
    let i: integer;
    for (i = 0; i < 10; i = i +2;)
    {
        print i, "\n";
    }
}
