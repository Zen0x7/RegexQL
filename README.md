> Regular Expressions are powerful but not human friendly.

# RegexQL

## Goal

Build a SQL-like solution to test human-readable expressions as regular expressions.

## Usages

### CLI

```
regexql --query="Are two alphanumeric groups separated by slash?" --input="/assets/css" --boolean
```

RegexQL should return true and eventually, print `/[a-z0-9]+/[0-9]` when `--verbose` exists.

### SDK

### JavaScript

```js
import regexql from "@regexql/lang"

const query = "Are two alphanumeric groups separated by slash?";
const input = "/assets/css";

const response = await regexql
    .test(
        query,
        input
    );

console.log(response)
// true
```