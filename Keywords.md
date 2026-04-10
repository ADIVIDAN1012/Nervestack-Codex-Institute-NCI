# Nervestack Language Keywords

This document provides a comprehensive dictionary of all keywords in the Nervestack language, organized by category.

---

## Core Declarations

| Keyword     | Meaning     | Description                                                                          |
| ----------- | ----------- | ------------------------------------------------------------------------------------ |
| `spec`      | Function    | Defines a function or method.                                                        |
| `blueprint` | Class       | Declares a class for creating objects.                                               |
| `bridge`    | Interface   | Defines a contract for interaction between components.                               |
| `shard`     | Property    | Declares a property or attribute within a `blueprint`.                               |
| `prep`      | Constructor | Defines the initializer method for a `blueprint`.                                    |
| `forward`   | Return      | Returns a value from a `spec`.                                                       |
| `firm`      | Constant    | Declares a variable whose value cannot be changed.                                   |
| `solid`     | Static      | Declares a variable or `spec` that belongs to the `blueprint` itself, not instances. |

---

## Control Flow

| Keyword     | Meaning     | Description                                               |
| ----------- | ----------- | --------------------------------------------------------- |
| `when`      | If          | Begins a conditional block.                               |
| `otherwise` | Else / Elif | A subsequent conditional block or the final fallback.     |
| `done`      | End Block   | Marks the end of a conditional, function, or loop block.  |
| `traverse`  | For Loop    | Iterates over a sequence or range.                        |
| `until`     | While Loop  | Executes a block as long as a condition is true.          |
| `halt`      | Break       | Exits a loop immediately.                                 |
| `proceed`   | Continue    | Skips to the next iteration of a loop.                    |
| `wait`      | Pass        | A no-op statement.                                        |

---

## Error Handling

| Keyword    | Meaning       | Description                                                     |
| ---------- | ------------- | --------------------------------------------------------------- |
| `attempt`  | Try           | Begins a block of code where errors will be handled.            |
| `trap`     | Catch         | Catches and handles a specific error.                           |
| `conclude` | Finally       | A block of code that always executes, regardless of errors.     |
| `trigger`  | Raise         | Manually raises an error.                                       |
| `Blame`    | Exception     | The base type for all errors.                                   |
| `peek`     | Error Message | A special variable in a `trap` block holding the error details. |

---

## Data & Types

| Keyword   | Meaning      | Description                                              |
| --------- | ------------ | -------------------------------------------------------- |
| `Num`     | Number       | The data type for integers and decimals.                 |
| `Text`    | String       | The data type for text.                                  |
| `On`      | True         | The boolean `true` value.                                |
| `Off`     | False        | The boolean `false` value.                               |
| `Nil`     | Null         | Represents the absence of a value.                       |
| `kind`    | Type Of      | A built-in function that returns the type of a variable. |
| `convert` | Type Casting | Converts a variable from one type to another.            |

---

## Modules & Scope

| Keyword    | Meaning   | Description                                                                      |
| ---------- | --------- | -------------------------------------------------------------------------------- |
| `toolkit`  | Module    | Defines a reusable module of code.                                               |
| `plug`     | Import    | Imports a `toolkit`.                                                             |
| `share`    | Export    | Makes a `spec`, `blueprint`, or `firm` variable available outside its `toolkit`. |
| `expose`   | Public    | Marks a `bridge` method as publicly accessible.                                  |
| `hidden`   | Private   | Restricts access to a `spec` or `shard` to within its defining `blueprint`.      |
| `shielded` | Protected | Restricts access to the defining `blueprint` and its children.                   |
| `internal` | Internal  | Restricts access to within the same `toolkit`.                                   |

---

## Object-Oriented Programming

| Keyword | Meaning  | Description                                                 |
| ------- | -------- | ----------------------------------------------------------- |
| `adopt` | Inherits | Specifies that a `blueprint` inherits from another.         |
| `own`   | Self     | Refers to the current object instance inside a `blueprint`. |
| `link`  | Bind     | Connects a variable to a `bridge` implementation.           |
| `embed` | Embed    | Includes one `bridge`'s interface within another.           |

---

## Comments

| Syntax      | Meaning                                             |
| ----------- | --------------------------------------------------- |
| `< ... >`   | Single-line comment.                                |
| `<^ ... ^>` | Multi-line comment.                                 |
| `note`      | Docstring for documenting `spec`s and `blueprint`s. |

---

## Concurrency

| Keyword  | Meaning  | Description                                        |
| -------- | -------- | -------------------------------------------------- |
| `paral`  | Parallel | Marks a `spec` to be run in parallel.              |
| `hold`   | Await    | Pauses execution until a `paral` `spec` completes. |
| `signal` | Signal   | Emits a signal for event-driven programming.       |
| `listen` | Listener | Listens for a `signal`.                            |

---

## Advanced & Miscellaneous

| Keyword     | Meaning     | Description                                                     |
| ----------- | ----------- | --------------------------------------------------------------- |
| `ask`       | Input       | A built-in function to get input from the user.                 |
| `show`      | Print       | A built-in function to display output.                          |
| `authen`    | Assert      | Checks if a condition is true, raising an error if not.         |
| `transform` | Map         | A functional keyword to apply a function to a sequence.         |
| `condense`  | Reduce      | A functional keyword to aggregate a sequence to a single value. |
| `pack`      | Serialize   | Converts an object into a storable/transmittable format.        |
| `unpack`    | Deserialize | Converts serialized data back into an object.                   |
