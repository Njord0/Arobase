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

```fn main() : void
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
