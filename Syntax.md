# Nervestack Syntax Guide

This document provides a complete reference to the syntax of the Nervestack programming language. Nervestack uses a block-based syntax with explicit `done` terminators.

---

## Basic Syntax

### Comments

Single-line and multi-line comments are used to add notes that are ignored by the compiler.

```nervestack
< This is a single-line comment >

<^
This is a multi-line comment.
It can span several lines.
^>
```

### Variable Declaration and Assignment

Variables are dynamically typed.

```nervestack
my_variable = "Hello, Nervestack"
user_age = 30
```

### Constants

Use `firm` to declare constants.

```nervestack
firm PI = 3.14159
```

### Output

The `show` keyword prints values. It supports multiple arguments and string interpolation.

```nervestack
show "Welcome to Nervestack!"
show "The value of PI is |PI|"
show "Sum:", 10 + 20
```

### Input

Use `ask` to get input from the user.

```nervestack
firm name = ask "What is your name? "
show "Hello, |name|!"
```

---

## Functions

Functions are declared with the `spec` keyword.

```nervestack
spec add with a, b giving Num:
    forward a + b
done

< Calling a function >
result = funcall add(10, 20)
```

Docstrings can be added using the `note` keyword inside a `spec`.

```nervestack
spec calculate_sum with a, b:
    note "This spec returns the sum of two numbers."
    forward a + b
done
```

---

## Control Flow

### Conditionals

`when`, `otherwise when`, and `otherwise` are used for conditional logic.

```nervestack
when x > 10:
    show "x is greater than 10"
otherwise when x == 10:
    show "x is exactly 10"
otherwise:
    show "x is less than 10"
done
```

### Loops

`traverse` and `until` are used for looping.

```nervestack
< For loop >
traverse i from 1 to 5:
    show "Iteration: |i|"
done

< While loop >
count = 0
until count >= 5:
    show "Count is |count|"
    count = count + 1
done
```

---

## Error Handling

The `attempt-trap-conclude` block is used for handling errors.

```nervestack
attempt:
    risky_operation()
trap SomeError:
    show "Caught an error: |peek|"
conclude:
    show "This block always executes."
done
```

---

## Object-Oriented Syntax

### Blueprints (Classes)

A `blueprint` defines the structure for an object.

```nervestack
blueprint Dog:
    shard name
    solid species = "Canine"

    prep with own, dog_name:
        own~>name = dog_name
    done

    spec bark with own:
        show "|own~>name| says woof!"
    done
done
```

### Inheritance

A `blueprint` can `adopt` from another.

```nervestack
blueprint Poodle:
    adopt Dog

    spec groom with own:
        show "|own~>name| is being groomed."
    done
done
```

### Objects

Create an instance with `spawn`. Use `~>` to access members.

```nervestack
my_dog = spawn Dog("Buddy")
my_dog~>bark()
```

---

## Modules

### Toolkits

A `toolkit` defines a reusable module.

```nervestack
toolkit Math:
    share spec add with a, b:
        forward a + b
    done
done
```

### Importing

Use `plug` to import a `toolkit`.

```nervestack
plug Math from "math_utils.ns"

sum = Math~>add(5, 10)
```
