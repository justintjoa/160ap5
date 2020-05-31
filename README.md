# CS 160 Assignment 5: Dataflow Analysis and Global Common Subexpression Elimination

**Assignment due: Tuesday, June 2 11:59 PM**

Read [The course description post on
Slack](https://cs160spring20.slack.com/archives/C010B82M9KL/p1585378023017500)
for late submission and slip day policy.

In this assignment you are going to implement an optimization known,
as "global common subexpression elimination" (GCSE), on top of an IR
that we provide.  This assignment consists of a single IR pass over
the AST and sits in the "middle end" of our C1 compiler. The output of
the optimization will be a linear sequence of code, much like the
output of the code gen assignment, with instructions drawan from our
IR grammar.

## What is provided

We provide you with an `IR` class, a `BasicBlock` class and a `CFG` class. The
`IR` class computes an intermediate representation over the AST created in
assignment 2.  The IR pass is described in lecture 37 and entails transforming
the AST into a form called "thre address code" (TAC).  This IR is then used to
construct a control flow graph, the code for which is also given to you.
Running just the assignment as it is should take an L1 program, lex it into a
token stream, parse the token stream into an AST, transform the AST into three
address code, and finally compute the control flow graph of the IR.  The CFG is
built on top of the `BasicBlock` and `CFG` classes.

The `generateCFG` method is the main entry point of the IR, taking an AST for
the whole program and returning vectors of instructions in three address code
for each function in the program, including the "global function" that consists
of statements after any function defs the L1 source.  It will additionally
populate a CFG object in the IR class representing the control flow graph for
each function definition in the program.

We will provide our solution once the slip day period for assignment 5
has passed, then you can use that code to help test your optimizer on
source code files. In the meantime, you can test your code using L1
source files or ASTs.  We also provide the driver code in `main.cpp`
that connects all components of our compiler and runs the optimizer.

## What you need to implement

You need to implement the `computeGCSE` and `computeAvailExprs` methods in the
`CFG` class.  For each control flow graph you will call `computeGCSE` which will
(presumably also call `computeAvailExprs`) to perform global common
subexpression elimination, possibly eliminating repeated instructions.

In order to implement the optimize method, you will need to implement your own
helper functions for computing the available expressions analysis outlined in
the IR lectures, including calculating gen/kill sets, as well as writing a
worklist algorithm, and additionally any functions related to GCSE itself.  The
GCSE output should be a sequence of IR code and so this will be graded just in
terms of string comparison, as in the diff described below.

Because this assignment is a chain of transformations on the AST, if you make a
mistake at one level the downstream transformations will likely also be wrong.
To this end, we will be grading both the output of the available expressions
analysis, as well as the final output of the GCSE.  The GCSE output should be a
sequence of IR code and so this will be graded just in terms of string
comparison.  The optimization requires you to generate fresh variable names,
such as `_opt1` similar to the tmpVars from the code gen (and this) assignment.
While that is not provided, you must conform to same `_optN` scheme, and further
number expressions in the set of all expressions in (typical) left-to-right
order.  The set of all expressions is computed similarly in a left-to-right
traversal of the vector of basic blocks for each function.  

The available expressions analysis is slightly more delicate.  You will
implement the gen and kill sets using vectors of booleans.  Every bool in the
vector corresponds to an expression in the block, and a bool is set if it is
available at that point in the program.  Thus if we have the following basic
blocks:

```
[A]
a <- 1 + d
b <- b - 2

[B]
c <- 3 - c
```
the set of all expressions would be a bitvector of width 3,
with the expression `1+b` as the first bit, `b-2` as the second,
and `3-4` as the third.  If we go through the analysis,
we will derive:

```
genA = 100   // only the first is generated
killA = 010  // kills everything involving a and b

genB = 000   // nothing generated
killB = 001  // expressions involving c killed

// inA and inB initialized to 000, outA and outB initialized to 111
// after worklist completion we get:
inA = 000 -> 000
outA = 111 -> 100

inB = 000 -> 100
outB = 111 -> 100
```

The available expressions will simply populate a field in the CFG class which we
will compare pointwise.  The only thing to keep in mind is that you may not
alter the way you traverse the CFG class data members, namely the basic blocks,
since that could result in a different boolean vector representation of the
expressions.


## How to test your optimizer

We included some test inputs and the unoptimized IR versions of those programs
in the `tests/` directory. There is additionally one optimized version.  There
is one expected optimized program currently, taken from lecture 39. You can
directly use the `diff` tool to compare the code generated by your optimizer to
the expected code.  More unit tests will be uploaded soon.

### Testing via GradeScope

Later this week, we will enable submission via GradeScope and your
assignments will be tested against a comprehensive and much larger
test suite. Auto-grader on GradeScope will give you feedback on your
assignment as well. You will be able to submit your assignment
multiple times and get feedback before submitting a final solution.

We will explain the details on the auto-grader once it is online.


## What to submit

You are going to submit your whole assignment through GradeScope. We
are going to consider only your changes to `midend/ir.cpp` and
`midend/ir.h` while grading and ignore changes in other files
and additional files. We are going to clarify submission instructions
once GradeScope is online.

Your submitted `ir.h` and `ir.cpp` need to compile with the
skeleton code given to you, otherwise you automatically fail the
assignment.
