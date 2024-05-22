> Regular Expressions are powerful but not human friendly.

# RegexQL

## Goal

Build a SQL-like solution to test human-readable expressions as regular expressions.

## Usages

### CLI

```bash
RegexQL --query="Are two alphanumeric groups separated by slash?" --input="/assets/css"
```

RegexQL should return true and eventually, print `/[a-z0-9]+/[0-9]` when `--verbose` exists.