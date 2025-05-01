# POSIX Examples

Practical POSIX thread and synchronization examples in C.

## Compilation

To compile `main.c`:
```terminal
gcc -o main main.c -lpthread
```

To run the program:
```terminal
./main
```

## Stack Analysis

To analyze function stack addresses:

1. Compile with map file generation:
```terminal
gcc -o main main.c -lpthread -Wl,-Map=main.map
```

2. Run the program:
```terminal
./main
```

3. Analyze the map file:
```terminal
cat main.map
```
