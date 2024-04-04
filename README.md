# C- Language Compiler by Ian Fleming

Here you will find the codebase for my language compiler made from scratch. The custom language this compiler was made for is called "C-". 
You can find the grammar and other information on the syntax and behavior of this language in the C- Grammar PDF in the 'final' branch.
This repository contains various implementations of programming concepts, principles, and techniques related to compiler design and development. 
This was assigned as a class project for my CS445 Compiler course at the University of Idaho in Spring 2023.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [License](#license)

## Features
This compiler project encompasses a wide range of features and techniques, including:

- Abstract Syntax Trees (AST)
- Lexical Analysis (Tokenization)
- Parsing (Syntax Analysis)
- Semantic Analysis
- Data Structures
- Breadth-First Search (BFS)
- Depth-First Search (DFS)
- Code Generation

## Installation

This project requires you to have ``a native gcc/g++ compiler, make, bison, and flex`` installed and working properly in your native environment.

1. Clone the repostiory

    ``git clone https://github.com/ibfleming/my-compiler.git``

## Usage

1. Compile the program.

    ``make clean;make``

    *This first cleans the old objects/artifacts from previous compilations and then compiles.*

2. Executing the program...

    Inside the **test** folder of the repository contains C- files that we can use to compile.
    You can execute the program as follows:
   
   ``./c- <c- file path>`` *(An input file is required!)*

    #### Arguments (Pass these into the execution of the program, example below...):
   
    - **M** - Prints the augmented tree of the code.
    - **p** - Prints the regular tree of the code.
    - **P** - Prints the annotated tree of the code.
    - **d** - Enables yydebug (YACC debug print-outs).
    - **#** - My custom debug for various random things.
    
   *Ex:* To print the annotated and augmented trees you would execute the following...
   
   ``./c- <c- file path> -P -M``

3. Enjoy!
    
## License

This project is licensed under the [MIT License](LICENSE). See the [LICENSE](LICENSE) file for details.
