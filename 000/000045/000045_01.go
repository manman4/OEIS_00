package main 

import "fmt"

func fibo(n int) int {
    x, y := 0, 1
    for i := 0; i < n; i++ {
        x, y = y, x+y
    }
    return x
}

func main() {
    fmt.Println(fibo(40))
}