# Project 1 — My Piggy Bank

**Student:** Souad Mostafa Kamel  
**Module:** Microcontroller-Based Systems (Plain C)  
**Environment:** C99 Compliant (`gcc -std=c99 -Wall -Wextra`)

---

## 1. Project Overview
This project implements an interactive digital piggy bank in standard C. It tracks coins of 5 denominations (25p, 50p, 100p, 200p, 500p), displays visual ASCII bars representing current savings, verifies whether the user can afford various toys, and produces a complete financial summary report.

---

## 2. Technical Highlights & Rule Compliance

* **Input Validation:** All user inputs are parsed safely via `readInt()` using `fgets()` and `sscanf()`. Typing invalid text prints an informative message without crashing or hanging in infinite loops.
* **Transaction Safety:** Coin withdrawals verify availability prior to updating counts; withdrawal requests exceeding current funds are safely rejected to prevent negative counts or underflow.
* **Pure Recursion (`sumCoins`):** The total coin summation across the bank is implemented recursively with no `for` or `while` loops, satisfying the assignment's recursive constraint.
* **Static Functions & Code Length:** Every function is declared `static` and kept under 40 lines of code. The `main()` function solely dispatches commands via a menu loop without directly modifying state data.

---

## 3. How to Build and Run

Compile the program with all warnings enabled:
```bash
gcc -std=c99 -Wall -Wextra -o app main.c
