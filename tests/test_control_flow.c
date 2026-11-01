int a;
int b;
int c;
int i;
int n;

// if / else
a = 3;
b = 7;
if (a < b) {
c = 1;
} else {
c = 0;
}

// while loop with a relational condition
i = 0;
n = 5;
while (i < n) {
i = i + 1;
}

// nested if-inside-while, to stress-test the backpatching stacks
while (i) {
if (b) {
c = 1;
} else {
c = 2;
}
i = 0;
}

return c;
