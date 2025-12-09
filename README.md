This repo is a [challenge by John Crickett](https://codingchallenges.fyi/challenges/challenge-json-parser/) 
to inspire us to learn about lexers, parsers, grammars and all compiler whatnots.

This is perhaps better called a JSON *validator*. I mean, it does actual parsing
but it lacks features of more robust implementations such as VS Code's builtin JSON linter.
In particular, it does not warn you where **and** which type of grammatical error the JSON
file presents. 

Actually, it only raises errors on the latter, showing the first lexical or
syntactic issue found, adopting a sort of "fail fast" principle as it 
is a LL(1) parser. In other words, the parser parses the text from **L**eft
to right, performs **L**eftmost derivation - which means it
expands the leftmost nonterminal before proceeding - and 
has a token lookahead of **1**.

This implementation is entirely compliant with the official JSON specification
([RFC 8259](https://www.rfc-editor.org/rfc/rfc8259.html)).

# Building
You need `gcc`, `make`, and probably build tools like Ubuntu's `build-essentials` 
or Arch's `base-devel`. Also, `valgrind` to optionally check for resource leaks.


# JSON?
To understand the formal grammar of the JavaScript Object Notation I highly recommend
reading the actual RFC, but I'll try to summarize it here.

A JSON text is a [UTF-8](https://en.wikipedia.org/wiki/UTF-8) text file of type:
```
ws value ws
```

where `ws` is whitespace to be ignored and `value` is the production:

$$ Value \rightarrow Number | String | Array ∣ Object ∣ Boolean | Null $$


## Numbers
A number is represented in base 10 with decimal digits. It has a mandatory integer
part (`int`) preceded by an optional `minus` sign for negatives.
It may also have a fractional component `frac`, which is a decimal point (`.`)
followed by digits. Finally it may have an exponential part (`exp`)
which starts with lower- or uppercase `e` followed by an optional
`minus` or `plus` sign and must have a sequence of digits afterwards.
Thus, the structure for a number is like:

`[-] int [frac] [exp]`


## Strings
These must start and end with quotation marks (`"`).
Inside of it lies zero or more characters (`char`), leaving us at:

`quotation-mark *char quotation-mark`

`char` can be a regular unescaped character or an escaped one.
In the latter case, it must be preceded by the backslash (`\`) and 
followed by either:
- `"`: same old quotation mark
- `\`: reverse solidus
- `/`: solidus
- `b`: backspace
- `f`: form feed
- `n`: line feed (or newline)
- `r`: carriage return
- `t`: horizontal tab
- `uXXXX`: for a character outside Unicode's [Basic Multilingual Plane](https://en.wikipedia.org/wiki/Plane_(Unicode)#Basic_Multilingual_Plane), you must escape `u`, followed by 4 hexadecimal digits that represent it

## Arrays
Ordered collection of elements. MUST
start with `[` and end with `]`.

Arrays are nonterminals described by the production:

$$Array \rightarrow [ [ Value *(, Value) ] ]$$


## Objects
Unordered collection of elements. MUST
start with `{` and end with `}`.

Objects are also nonterminals described by the production:

$$Object \rightarrow \lbrace [ Member *(, Member)] \rbrace$$

where `Member` is:

$$Member \rightarrow String : Value$$

