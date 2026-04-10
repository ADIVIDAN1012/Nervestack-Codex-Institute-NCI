# 📜 Nervestack Syntax in 5 Minutes
**Easiest to Learn. Fastest to Run.**

Nervestack uses a block-based syntax with explicit `done` terminators. It's designed to be readable as plain English.

---

## 1. The Basics

### Comments
No cryptic symbols. Just use angle brackets.
```nervestack
< This is a single-line comment >

<^
This is a multi-line comment.
It's as easy as that.
^>
```

### Variables & Constants
Variables are dynamic. Use `firm` for constants.
```nervestack
age = 25
name = "Nervestack"
firm PI = 3.14
```

### Input & Output
`ask` and `show` make interaction intuitive.
```nervestack
user_name = ask "What's your name? "
show "Hello, |user_name|!"
```

---

## 2. Control Flow

### If-Else (When-Otherwise)
```nervestack
when score > 90:
    show "Excellent!"
otherwise when score > 50:
    show "Good job."
otherwise:
    show "Try again."
done
```

### Loops (Traverse & Until)
```nervestack
< Count from 1 to 5 >
traverse i from 1 to 5:
    show "Step: |i|"
done

< While loop >
count = 0
until count >= 5:
    show "Count: |count|"
    count = count + 1
done
```

---

## 3. Functions (Spec)
Functions are "Specifications" of behavior. Use `with` for parameters and `giving` for return types.
```nervestack
spec add with a, b giving Num:
    forward a + b
done

result = funcall add(5, 10)
```

---

## 4. Object-Oriented (Blueprint)
Nervestack uses the `blueprint` metaphor for classes.
```nervestack
blueprint Car:
    shard model
    
    prep with own, car_model:
        own~>model = car_model
    done
    
    spec drive with own:
        show "Driving the |own~>model|..."
    done
done

my_car = spawn Car("Nervestack-Z1")
my_car~>drive()
```

---

## 5. Error Handling (Attempt)
A narrative flow for safety.
```nervestack
attempt:
    risky_logic()
trap ConnectionError:
    show "Lost connection: |peek|"
conclude:
    show "Cleaning up..."
done
```

---

## 🚀 Why it's the Fastest?
Every instruction is parsed by our Python frontend and executed by our custom **C++ Runtime Engine**, giving you high-level syntax with low-level speed.
